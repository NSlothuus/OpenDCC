// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/texture_paint/texture_data.h"
#include "OpenImageIO/imageio.h"
#include "OpenImageIO/imagebufalgo.h"
#include "opendcc/app/viewport/texture_plugin.h"
#include <pxr/usd/ar/resolver.h>

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN
using namespace OIIO;
namespace
{
    static constexpr int UDIM_START = 1001;
    static constexpr int UDIM_END = 1100;
    static const std::string s_udim_pattern = "<UDIM>";
};

std::unique_ptr<ImageData> ImageData::make_image(const std::string& file, uint32_t udim_index)
{
    auto img_input = ImageInput::open(file);
    if (!img_input)
        return nullptr;
    const auto& spec = img_input->spec();
    const auto desired_format = TypeDesc(TypeDesc::BASETYPE::UINT8, TypeDesc::AGGREGATE::SCALAR, 0);
    std::vector<uint8_t> pixels(spec.width * spec.height * spec.nchannels);
    const auto scanline_stride = spec.width * spec.nchannels * sizeof(uint8_t);
    if (!img_input->read_image(0, 0, 0, spec.nchannels, desired_format, pixels.data() + (spec.height - 1) * scanline_stride, OIIO::AutoStride,
                               -scanline_stride))
    {
        return nullptr;
    }

    std::shared_ptr<ImageBuf> img_buf = std::make_shared<ImageBuf>();
    ImageBuf src_img_buf(spec, pixels.data());
    auto result = std::make_unique<ImageData>();
    result->src_descr = src_img_buf.spec().format;
    result->src_channels = spec.nchannels;
    if (result->src_channels != 4)
    {
        int channel_order[4];
        float channel_values[] = { 0, 0, 0, 1 };
        std::string channel_names[] = { "R", "G", "B", "A" };
        for (int i = 0; i < 4; i++)
            channel_order[i] = i < src_img_buf.spec().nchannels ? i : -1;

        *img_buf = ImageBufAlgo::channels(src_img_buf, 4, channel_order, channel_values, channel_names);
        if (!img_buf->pixels_valid())
            return nullptr;
    }
    else
    {
        img_buf->copy(src_img_buf);
    }

    const auto tex_name = udim_index == 0 ? "texblock://painted_texture.wtex" : "texblock://painted_texture_" + std::to_string(udim_index) + ".wtex";
    InMemoryTextureRegistry::instance().add_texture(tex_name, img_buf);
    result->texture_buffer = img_buf;
    result->size = result->texture_buffer->spec().image_bytes();
    result->dims = GfVec2i(spec.width, spec.height);
    result->format = HgiFormatUNorm8Vec4;
    result->texture_file = file;
    result->udim_index = udim_index;

    result->shared_px_data.resize(spec.width * spec.height);
    for (int y = 0; y < result->dims[1]; ++y)
    {
        for (int x = 0; x < result->dims[0]; ++x)
        {
            const auto ind = y * spec.width + x;
            auto& px_data = result->shared_px_data[ind];
            px_data.img_data = result.get();
            result->texture_buffer->getpixel(x, y, px_data.orig_color.data());
            px_data.x = x;
            px_data.y = y;
        }
    }

    return result;
}

ImageData::~ImageData()
{
    if (writing_worker.joinable())
        writing_worker.join();
}

void ImageData::write()
{
    {
        auto out = ImageOutput::create(texture_file);
        ImageSpec out_spec(dims[0], dims[1], src_channels, src_descr);
        int scanline_size = out_spec.scanline_bytes();
        if (src_channels != texture_buffer->spec().nchannels)
        {
            auto dst = ImageBufAlgo::channels(*texture_buffer, src_channels, {});
            if (out->open(texture_file, out_spec))
                out->write_image(src_descr, (uint8_t*)dst.localpixels() + (out_spec.height - 1) * scanline_size, AutoStride, -scanline_size);
        }
        else
        {
            if (out->open(texture_file, out_spec))
                out->write_image(src_descr, (uint8_t*)texture_buffer->localpixels() + (out_spec.height - 1) * scanline_size, AutoStride,
                                 -scanline_size);
        }
    }
}

TextureData::TextureData(const std::string& texture_filename)
{
    const auto udim_start = texture_filename.find(s_udim_pattern);
    if (udim_start != std::string::npos)
    {
        const auto prefix = texture_filename.substr(0, udim_start);
        const auto suffix = texture_filename.substr(udim_start + s_udim_pattern.length());

        for (int i = UDIM_START; i < UDIM_END; i++)
        {
            const auto udim_path = prefix + std::to_string(i) + suffix;
            const auto resolved_path = ArGetResolver().Resolve(udim_path);
            if (!resolved_path.empty())
            {
                auto image_data = ImageData::make_image(prefix + std::to_string(i) + suffix, i);
                if (image_data)
                {
                    m_image_data[i] = std::move(image_data);
                    if (m_image_data.size() == 1) // is first tile
                    {
                        m_tex_dimensions = m_image_data[i]->dims;
                        m_is_udim = true;
                    }
                }
            }
        }
    }
    else if (auto image_data = ImageData::make_image(texture_filename))
    {
        image_data->udim_index = UDIM_START;
        m_image_data[UDIM_START] = std::move(image_data);
        m_tex_dimensions = m_image_data[UDIM_START]->dims;
        m_is_udim = false;
    }

    if (m_image_data.empty())
        clear();
    else
        m_texture_filename = texture_filename;
}

const GfVec2i& TextureData::get_dimensions() const
{
    return m_tex_dimensions;
}

const std::unordered_map<int, std::unique_ptr<ImageData>>& TextureData::get_image_data() const
{
    return m_image_data;
}

const std::string& TextureData::get_texture_filename() const
{
    return m_texture_filename;
}

bool TextureData::is_udim() const
{
    return m_is_udim;
}

bool TextureData::is_valid() const
{
    return !m_image_data.empty();
}

void TextureData::flush()
{
    for (auto& image : m_image_data)
    {
        if (image.second->dirty)
        {
            image.second->write();
            image.second->dirty = false;
        }
    }
}

void TextureData::invalidate()
{
    for (auto& img : m_image_data)
    {
        for (auto& px_data : img.second->shared_px_data)
        {
            px_data.img_data->texture_buffer->getpixel(px_data.x, px_data.y, px_data.orig_color.data());
            px_data.influence = 0.0f;
            px_data.touched = false;
        }
    }
}

void TextureData::clear()
{
    m_texture_filename.clear();
    m_image_data.clear();
}
OPENDCC_NAMESPACE_CLOSE
