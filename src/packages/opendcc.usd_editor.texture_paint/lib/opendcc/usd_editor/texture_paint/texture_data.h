/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/vec2i.h>
#include <pxr/imaging/hio/image.h>
#include <pxr/imaging/hgi/types.h>
#include <OpenImageIO/imagebuf.h>

OPENDCC_NAMESPACE_OPEN

struct ImageData;
struct SharedPixelData
{
    ImageData* img_data = nullptr;
    PXR_NS::GfVec4f orig_color;
    float influence = 0;
    int x = -1;
    int y = -1;
    bool touched = false;
};
struct ImageData
{
    std::string texture_file;
    std::shared_ptr<OIIO::ImageBuf> texture_buffer;
    std::vector<SharedPixelData> shared_px_data;
    OIIO::TypeDesc src_descr;
    PXR_NS::GfVec2i dims;
    int src_channels;
    PXR_NS::HgiFormat format;
    PXR_NS::HioFormat hio_format;
    std::mutex writing_mutex;
    std::thread writing_worker;
    size_t size;
    PXR_NS::HioImageSharedPtr out_file;
    int udim_index = 0;
    bool dirty = false;

    ~ImageData();
    void write();
    static std::unique_ptr<ImageData> make_image(const std::string& file, uint32_t udim_index = 0);
};

class TextureData
{
public:
    TextureData(const std::string& texture_filename);

    // Assumes all images have equal dimensions.
    const PXR_NS::GfVec2i& get_dimensions() const;
    const std::unordered_map<int, std::unique_ptr<ImageData>>& get_image_data() const;
    const std::string& get_texture_filename() const;
    bool is_udim() const;
    bool is_valid() const;
    void flush();
    void invalidate();
    void clear();

private:
    std::string m_texture_filename;
    PXR_NS::GfVec2i m_tex_dimensions;
    std::unordered_map<int, std::unique_ptr<ImageData>> m_image_data;
    bool m_is_udim = false;
};

OPENDCC_NAMESPACE_CLOSE
