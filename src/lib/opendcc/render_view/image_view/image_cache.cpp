// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "image_cache.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRunnable>
#include <QDateTime>
#include <QThreadPool>
#include <QCoreApplication>

OPENDCC_NAMESPACE_OPEN

static std::string make_temporary_file_path(const std::string& scratch_image_location, const uint32_t key)
{
    std::string path = scratch_image_location.c_str();
    path += "/render_view.";
    path += std::to_string(QCoreApplication::applicationPid());
    path += ".";
    path += std::to_string(key);
    path += ".tif";
    return path;
}

//////////////////////////////////////////////////////////////////////////
// ImageSaveTask
//////////////////////////////////////////////////////////////////////////

class ImageSaveTask : public QRunnable
{
public:
    ImageSaveTask(RenderViewInternalImageCache* image_cache, OIIO::ImageBuf* image, const std::string& path)
        : m_image_cache(image_cache)
        , m_image(image)
        , m_path(path)
    {
    }

    void run()
    {
        if (m_image)
        {
            {
                std::lock_guard<std::recursive_mutex> lk(m_image_cache->m_mutex);
                QFileInfo check_existing_file(m_path.c_str());
                if (check_existing_file.exists() && check_existing_file.isFile())
                    m_image_cache->m_disk_memory -= check_existing_file.size();
            }

            m_image->write(m_path.c_str());
            delete m_image;

            {
                std::lock_guard<std::recursive_mutex> lk(m_image_cache->m_mutex);
                QFileInfo check_created_file(m_path.c_str());
                if (check_created_file.exists())
                    m_image_cache->m_disk_memory += check_created_file.size();
                else
                    printf("ImageSaveTask error, fail to create file :%s\n", m_path.c_str());
            }
        }
    }

private:
    RenderViewInternalImageCache* m_image_cache;
    OIIO::ImageBuf* m_image;
    std::string m_path;
};

//////////////////////////////////////////////////////////////////////////
// RenderViewInternalImageCache
//////////////////////////////////////////////////////////////////////////

RenderViewInternalImageCache::RenderViewInternalImageCache(const int max_size)
{
    m_max_size = max_size;
    m_scratch_image_location = QDir::tempPath().toLocal8Bit().constData();
}

RenderViewInternalImageCache::~RenderViewInternalImageCache()
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    for (auto& image : m_image_list)
    {
        if (image.type == RenderViewImageType::Internal)
        {
            const std::string file_path = make_temporary_file_path(m_scratch_image_location, image.image_id);
            QFileInfo checkFile(file_path.c_str());
            if (checkFile.exists() && checkFile.isFile())
            {
                QFile::remove(file_path.c_str());
            }
        }
    }
}

bool RenderViewInternalImageCache::put(OIIO::ImageBuf* buf, uint32_t& key)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    if (!free_memory_for_image())
    {
        return false;
    }

    RenderViewImage new_img;
    new_img.buf = buf;
    new_img.active = true;
    new_img.type = RenderViewImageType::Internal;

    new_img.image_spec = buf->spec();
    new_img.acquired_count = 0;
    new_img.deleted = false;
    key = ++m_counter;

    new_img.image_id = key;

    m_image_list.push_front(new_img);
    m_image_map[key] = m_image_list.begin();
    m_allocated_memory += compute_image_size(buf->spec());
    return true;
}

bool RenderViewInternalImageCache::put_external(const char* filepath, uint32_t& key)
{
    auto input = OIIO::ImageInput::open(filepath);
    if (!input)
        return false;

    RenderViewImage new_img;
    new_img.buf = nullptr;
    new_img.active = false;
    new_img.type = RenderViewImageType::External;
    new_img.file_path = std::string(filepath);
    new_img.image_spec = input->spec();
    new_img.acquired_count = 0;
    new_img.deleted = false;

    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    key = ++m_counter;
    new_img.image_id = key;
    m_image_list.push_front(new_img);
    m_image_map[key] = m_image_list.begin();

    input->close();

    return true;
}

void RenderViewInternalImageCache::delete_image(const uint32_t key)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    auto image_it = m_image_map.find(key);
    if (image_it != m_image_map.end())
    {
        auto image = image_it->second;

        if (image->acquired_count > 0)
        {
            image->deleted = true;
        }
        else
        {
            delete_on_disk(key);
        }
    }
}

bool RenderViewInternalImageCache::free_memory_for_image()
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);

    if (m_image_list.size() < m_max_size)
    {
        return true;
    }

    auto in_memory = compute_in_memory_count();

    const auto begin = m_image_list.begin();
    auto image = m_image_list.end();

    while (in_memory >= m_max_size)
    {
        --image;
        if (!image->active || image->acquired_count >= 1)
        {
            continue;
        }

        if (image->type == RenderViewImageType::Internal)
        {
            image->file_path = make_temporary_file_path(m_scratch_image_location, image->image_id);
            ImageSaveTask* task = new ImageSaveTask(this, image->buf, image->file_path);
            task->setAutoDelete(true);
            QThreadPool::globalInstance()->start(task);
            image->buf = nullptr;
        }

        // added that in attempt to fix a lot crash reports
        // that may mean that we have some memory leaks
        if (image->buf)
        {
            delete image->buf;
        }

        image->buf = nullptr;
        image->active = false;
        m_allocated_memory = m_allocated_memory - compute_image_size(image->image_spec);
        --in_memory;

        if (image == begin)
        {
            break;
        }
    }

    return in_memory < m_max_size;
}

size_t RenderViewInternalImageCache::compute_image_size(OIIO::ImageSpec spec)
{
    return spec.image_bytes();
}

bool RenderViewInternalImageCache::get_spec(const uint32_t key, OIIO::ImageSpec& spec)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    auto image_it = m_image_map.find(key);
    if (image_it != m_image_map.end())
    {
        auto image = image_it->second;

        spec = image->image_spec;
        return true;
    }
    return false;
}

OIIO::ImageBuf* RenderViewInternalImageCache::acquire_image(const uint32_t key)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);

    auto image_it = m_image_map.find(key);
    if (image_it != m_image_map.end())
    {
        auto image = image_it->second;

        if (image->active)
        {
            ++image->acquired_count;
            m_image_list.splice(m_image_list.begin(), m_image_list, image_it->second);
            return image->buf;
        }
        else
        {
            const size_t image_size = compute_image_size(image->image_spec);
            if (!free_memory_for_image())
            {
                return nullptr;
            }

            auto input = OIIO::ImageInput::open(image->file_path);
            if (!input)
            {
                return nullptr;
            }

            OIIO::ImageBuf* buf = new OIIO::ImageBuf(image->file_path, image->image_spec);

            auto localpixels = buf->localpixels();
            if (!localpixels)
            {
                printf("ImageBuf error, localpixels is empty :%s\n", image->file_path.c_str());
                return nullptr;
            }

            input->read_image(image->image_spec.format, localpixels);
            input->close();

            image->active = true;
            image->buf = buf;
            m_allocated_memory += image_size;

            ++image->acquired_count;
            m_image_list.splice(m_image_list.begin(), m_image_list, image_it->second);
            return buf;
        }
    }
    return nullptr;
}

void RenderViewInternalImageCache::release_image(const uint32_t key)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    auto image_it = m_image_map.find(key);
    if (image_it != m_image_map.end())
    {
        auto image = image_it->second;

        if (image->acquired_count > 0)
        {
            --image->acquired_count;
        }

        m_image_list.splice(m_image_list.begin(), m_image_list, image_it->second);
        if (image->deleted)
        {
            delete_on_disk(image->image_id);
        }
    }
}

bool RenderViewInternalImageCache::update_spec(const uint32_t key, OIIO::ImageSpec spec)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    auto image_it = m_image_map.find(key);
    if (image_it != m_image_map.end())
    {
        auto image = image_it->second;

        image->image_spec = spec;
        return true;
    }
    return false;
}

bool RenderViewInternalImageCache::exist(const uint32_t key)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    auto image_it = m_image_map.find(key);
    if (image_it != m_image_map.end())
    {
        return true;
    }
    return false;
}

void RenderViewInternalImageCache::print_actual_cache_size(const char* step)
{
    size_t actual_memory = 0;

    for (auto& image : m_image_list)
    {
        if (image.active && image.buf)
        {
            actual_memory += image.buf->spec().image_bytes();
        }
    }
    printf("%zuMB         |%s\n", actual_memory / 1024u / 1024u, step);
}

size_t RenderViewInternalImageCache::compute_in_memory_count()
{
    int on_disk_count = 0;
    for (const auto& image : m_image_list)
    {
        const bool on_disk = !image.buf && !image.active;
        on_disk_count += static_cast<int>(on_disk);
    }
    return m_image_list.size() - on_disk_count;
}

void RenderViewInternalImageCache::delete_on_disk(const int32_t key)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    auto image_it = m_image_map.find(key);
    if (image_it != m_image_map.end())
    {
        auto image = image_it->second;

        if (image->acquired_count <= 0)
        {
            const std::string file_path = make_temporary_file_path(m_scratch_image_location, image->image_id);
            QFileInfo checkFile(file_path.c_str());
            if (image->type == RenderViewImageType::Internal && checkFile.exists() && checkFile.isFile())
            {
                bool result = QFile::remove(file_path.c_str());
                if (result)
                    m_disk_memory -= checkFile.size();
                else
                    printf("failed to delete file %s", file_path.c_str());
            }

            if (image->active)
            {
                delete image->buf;
                image->buf = nullptr;
            }

            m_allocated_memory = m_allocated_memory - compute_image_size(image->image_spec);

            m_image_list.erase(image_it->second);
            m_image_map.erase(image_it);
        }
    }
}

void RenderViewInternalImageCache::set_scratch_image_location(const std::string& scratch_image_location)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    m_scratch_image_location = scratch_image_location;
}

std::string RenderViewInternalImageCache::get_file_path(const uint32_t key)
{
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    std::string result;
    auto image_it = m_image_map.find(key);
    if (image_it != m_image_map.end())
    {
        auto image = image_it->second;

        return image->file_path;
    }

    return result;
}

void RenderViewInternalImageCache::used_memory(uint64_t& allocated, uint64_t& disk) const
{
    allocated = m_allocated_memory;
    disk = m_disk_memory;
}

void RenderViewInternalImageCache::set_max_size(const int max_size)
{
    m_max_size = max_size;
    free_memory_for_image();
}

OPENDCC_NAMESPACE_CLOSE
