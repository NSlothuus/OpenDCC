/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <OpenImageIO/imagebuf.h>
#include <pxr/pxr.h>
#if PXR_VERSION < 2108
#include <pxr/imaging/glf/image.h>
#else
#include <pxr/imaging/hio/image.h>
#endif

OPENDCC_NAMESPACE_OPEN

#if PXR_VERSION < 2108
using PxrImageBase = PXR_NS::GlfImage;
template <class T>
using PxrImageFactory = PXR_NS::GlfImageFactory<T>;
#else
using PxrImageBase = PXR_NS::HioImage;
template <class T>
using PxrImageFactory = PXR_NS::HioImageFactory<T>;
#endif

class OPENDCC_API InMemoryTextureRegistry
{
public:
    static InMemoryTextureRegistry& instance();

    void add_texture(const std::string& path, std::shared_ptr<OIIO::ImageBuf> buffer);
    void remove_texture(const std::string& path);

    std::shared_ptr<OIIO::ImageBuf> get_texture(const std::string& path) const;

private:
    InMemoryTextureRegistry() = default;
    InMemoryTextureRegistry(const InMemoryTextureRegistry&) = delete;
    InMemoryTextureRegistry(InMemoryTextureRegistry&&) = delete;
    InMemoryTextureRegistry& operator=(const InMemoryTextureRegistry&) = delete;
    InMemoryTextureRegistry& operator=(InMemoryTextureRegistry&&) = delete;

    using Lock = std::lock_guard<std::mutex>;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<OIIO::ImageBuf>> m_texture_cache;
};

class OPENDCC_API InMemoryTexture : public PxrImageBase
{
public:
    InMemoryTexture();
    ~InMemoryTexture() override;

    virtual bool Read(StorageSpec const& storage) override;
    virtual bool ReadCropped(int const cropTop, int const cropBottom, int const cropLeft, int const cropRight, StorageSpec const& storage) override;
    virtual bool Write(StorageSpec const& storage, PXR_NS::VtDictionary const& metadata = PXR_NS::VtDictionary()) override;
    virtual std::string const& GetFilename() const override;
    virtual int GetWidth() const override;
    virtual int GetHeight() const override;
#if PXR_VERSION < 2108
    virtual GLenum GetFormat() const override;
    virtual GLenum GetType() const override;
#else
    virtual PXR_NS::HioFormat GetFormat() const override;
#endif
    virtual int GetBytesPerPixel() const override;
    virtual int GetNumMipLevels() const override;
    virtual bool IsColorSpaceSRGB() const override;
    virtual bool GetMetadata(PXR_NS::TfToken const& key, PXR_NS::VtValue* value) const override;
#if PXR_VERSION < 2108
    virtual bool GetSamplerMetadata(GLenum pname, PXR_NS::VtValue* param) const override;
#else
    virtual bool GetSamplerMetadata(PXR_NS::HioAddressDimension dim, PXR_NS::HioAddressMode* param) const override;
#endif

protected:
#if PXR_VERSION < 2108
    virtual bool _OpenForReading(std::string const& filename, int subimage, int mip, bool suppressErrors) override;
#else
    virtual bool _OpenForReading(std::string const& filename, int subimage, int mip, SourceColorSpace sourceColorSpace, bool suppressErrors) override;
#endif
    virtual bool _OpenForWriting(std::string const& filename) override;

private:
    std::weak_ptr<OIIO::ImageBuf> m_buf;
    std::string m_filename;
};

OPENDCC_NAMESPACE_CLOSE
