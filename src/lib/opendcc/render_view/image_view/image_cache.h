/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <OpenImageIO/imagebuf.h>

#include <list>
#include <mutex>
#include <atomic>
#include <string>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

class ImageSaveTask;

enum class RenderViewImageType
{
    Internal,
    External
};

struct RenderViewImage
{
    OIIO::ImageBuf* buf;
    std::string file_path;
    RenderViewImageType type;
    uint32_t image_id;

    OIIO::ImageSpec image_spec;
    bool active;

    bool deleted;
    uint32_t acquired_count;
};

class RenderViewInternalImageCache
{
    friend ImageSaveTask;

public:
    RenderViewInternalImageCache(const int max_size);
    ~RenderViewInternalImageCache();

    bool put(OIIO::ImageBuf* buf, uint32_t& key);

    bool put_external(const char* filepath, uint32_t& key);

    bool get_spec(const uint32_t key, OIIO::ImageSpec& spec);

    OIIO::ImageBuf* acquire_image(const uint32_t key);
    bool exist(const uint32_t key);
    bool update_spec(const uint32_t key, OIIO::ImageSpec spec);

    void release_image(const uint32_t key);

    void delete_image(const uint32_t key);
    void set_scratch_image_location(const std::string& scratch_image_location);

    std::string get_file_path(const uint32_t key);
    void used_memory(uint64_t& allocated, uint64_t& disk) const;

    void set_max_size(const int max_size);

private:
    void delete_on_disk(const int32_t key);

    bool free_memory_for_image();

    size_t compute_image_size(OIIO::ImageSpec spec);

    void print_actual_cache_size(const char* step);

    size_t compute_in_memory_count();

    // https://www.nextptr.com/tutorial/ta1576645374/stdlist-splice-for-implementing-lru-cache
    std::list<RenderViewImage> m_image_list;
    std::unordered_map<uint32_t, std::list<RenderViewImage>::iterator> m_image_map;

    uint64_t m_allocated_memory = 0;
    uint64_t m_disk_memory = 0;
    uint32_t m_counter = 0;
    int m_max_size;

    std::string m_scratch_image_location;
    std::recursive_mutex m_mutex;
};

OPENDCC_NAMESPACE_CLOSE
