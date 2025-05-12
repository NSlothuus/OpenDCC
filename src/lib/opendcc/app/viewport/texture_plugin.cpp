// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <pxr/pxr.h>
#if PXR_VERSION < 2108
#include <GL/glew.h>
#include <pxr/imaging/glf/glew.h>
#endif
#include "texture_plugin.h"
#include "OpenImageIO/imagebufalgo.h"

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN

TF_REGISTRY_FUNCTION(TfType)
{
    auto type = TfType::Define<InMemoryTexture, TfType::Bases<PxrImageBase>>();
    type.SetFactory<PxrImageFactory<InMemoryTexture>>();
}

InMemoryTexture::InMemoryTexture() {}

InMemoryTexture::~InMemoryTexture() {}

bool InMemoryTexture::Read(const StorageSpec& storage)
{
    return ReadCropped(0, 0, 0, 0, storage);
}

bool InMemoryTexture::_OpenForWriting(std::string const& filename)
{
    return false;
}

#if PXR_VERSION >= 2108
bool InMemoryTexture::_OpenForReading(std::string const& filename, int subimage, int mip, SourceColorSpace sourceColorSpace, bool suppressErrors)
#else
bool InMemoryTexture::_OpenForReading(std::string const& filename, int subimage, int mip, bool suppressErrors)
#endif
{
    if (mip != 0)
        return false;

    if (auto tex = InMemoryTextureRegistry::instance().get_texture(filename))
    {
        m_buf = tex;
        m_filename = filename;
        return true;
    }

    m_filename.clear();
    m_buf.reset();
    return false;
}
#if PXR_VERSION >= 2108
bool InMemoryTexture::GetSamplerMetadata(HioAddressDimension dim, HioAddressMode* param) const
#else
bool InMemoryTexture::GetSamplerMetadata(GLenum pname, VtValue* param) const
#endif
{
    return false;
}

bool InMemoryTexture::GetMetadata(TfToken const& key, VtValue* value) const
{
    return false;
}

bool InMemoryTexture::IsColorSpaceSRGB() const
{
    return false;
}

int InMemoryTexture::GetNumMipLevels() const
{
    return 1;
}

int InMemoryTexture::GetBytesPerPixel() const
{
    if (!m_buf.expired())
        return m_buf.lock()->spec().pixel_bytes();
    return 0;
}
#if PXR_VERSION >= 2108
HioFormat InMemoryTexture::GetFormat() const
{
    if (m_buf.expired())
        return HioFormat::HioFormatUNorm8;
    switch (m_buf.lock()->spec().nchannels)
    {
    case 4:
        return HioFormatUNorm8Vec4;
    case 3:
        return HioFormatUNorm8Vec3;
    case 2:
        return HioFormatUNorm8Vec2;
    default:
        return HioFormatUNorm8;
    }
}
#else

GLenum InMemoryTexture::GetFormat() const
{
    if (m_buf.expired())
        return 1;
    switch (m_buf.lock()->spec().nchannels)
    {
    case 1:
        return GL_RED;
    case 2:
        return GL_RG;
    case 3:
        return GL_RGB;
    case 4:
        return GL_RGBA;
    default:
        return 1;
    }
}

GLenum InMemoryTexture::GetType() const
{
    return GL_UNSIGNED_BYTE;
}
#endif

int InMemoryTexture::GetHeight() const
{
    if (m_buf.expired())
        return 0;
    return m_buf.lock()->spec().height;
}

int InMemoryTexture::GetWidth() const
{
    if (m_buf.expired())
        return 0;
    return m_buf.lock()->spec().width;
}

std::string const& InMemoryTexture::GetFilename() const
{
    return m_filename;
}

bool InMemoryTexture::Write(StorageSpec const& storage, VtDictionary const& metadata /*= VtDictionary()*/)
{
    return false;
}

bool InMemoryTexture::ReadCropped(int const cropTop, int const cropBottom, int const cropLeft, int const cropRight, StorageSpec const& storage)
{
    // Probably we should output some color for debugging purposes
    if (m_buf.expired())
        return false;

    auto buf = m_buf.lock();
    OIIO::ImageBuf* result = buf.get();

    OIIO::ImageBuf cropped;
    if (cropTop || cropBottom || cropLeft || cropRight)
    {
        OIIO::ImageBufAlgo::cut(cropped, *result, OIIO::ROI(cropLeft, buf->spec().width - cropRight, cropTop, buf->spec().height - cropBottom));
        result = &cropped;
    }

    // We don't support mipmaping and GetNumMipLevels always returns 1. But Storm UDIM texture object doesn't care,
    // it still tries to load mipmaped images, since it's readonly call, we can easily resample buffer to the desired
    // mip level.
    OIIO::ImageBuf formatted;
    if (storage.width != buf->spec().width || storage.height != buf->spec().height)
    {
        OIIO::ImageBufAlgo::resample(formatted, *result, false, OIIO::ROI(0, storage.width, 0, storage.height));
        result = &formatted;
    }

    if (!result->get_pixels(OIIO::ROI(0, storage.width, 0, storage.height, 0, 1), result->spec().format, storage.data))
        return false;

    return true;
}

InMemoryTextureRegistry& InMemoryTextureRegistry::instance()
{
    static InMemoryTextureRegistry instance;
    return instance;
}

std::shared_ptr<OIIO::ImageBuf> InMemoryTextureRegistry::get_texture(const std::string& path) const
{
    Lock lock(m_mutex);
    auto it = m_texture_cache.find(path);
    return it != m_texture_cache.end() ? it->second : nullptr;
}

void InMemoryTextureRegistry::remove_texture(const std::string& path)
{
    Lock lock(m_mutex);
    m_texture_cache.erase(path);
}

void InMemoryTextureRegistry::add_texture(const std::string& path, std::shared_ptr<OIIO::ImageBuf> buffer)
{
    Lock lock(m_mutex);
    m_texture_cache[path] = buffer;
}

OPENDCC_NAMESPACE_CLOSE
