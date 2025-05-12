/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <string>
#include <vector>
#include <stdint.h>
#include <map>
#include <memory>
#include <opendcc/render_view/display_driver_api/api.h>

OPENDCC_NAMESPACE_OPEN

namespace display_driver_api
{
    struct ROI
    {
        uint32_t xstart;
        uint32_t xend;
        uint32_t ystart;
        uint32_t yend;
    };

    enum struct ImageType
    {
        Unknown,
        Byte,
        UInt,
        Int,
        Float,
        HalfFloat
    };

    struct ImageDescription
    {
        int32_t parent_image_id;
        std::string image_name;
        uint32_t width;
        uint32_t height;
        uint32_t num_channels;
        ImageType image_data_type;
        std::map<std::string, std::string> extra_attributes;
    };

    struct Message
    {
        int32_t image_id;

        enum struct Type
        {
            Unknown,
            OpenImage,
            ActivateRegion,
            WriteRegion,
            CloseImage,
        } type;

        ImageDescription image_desc;

        ROI region;

        std::vector<char> bucket_data;
    };

    class RenderViewConnectionImpl;

    class RenderViewConnection
    {
    public:
        OPENDCC_RENDER_VIEW_API RenderViewConnection();
        OPENDCC_RENDER_VIEW_API virtual ~RenderViewConnection();
        OPENDCC_RENDER_VIEW_API void send_msg(Message& msg);
        OPENDCC_RENDER_VIEW_API int32_t recv_msg();

    private:
        std::unique_ptr<RenderViewConnectionImpl> m_impl;
    };

    OPENDCC_RENDER_VIEW_API void save_msg_to_buffer(const Message& msg, std::vector<char>& buffer);
    OPENDCC_RENDER_VIEW_API void load_msg_from_buffer(Message& msg, const std::vector<char>& buffer);

    // return id; if image_id == -1 create new, else swap existing; if parent_image = -1 on new image parent to nothing
    OPENDCC_RENDER_VIEW_API int32_t render_view_open_image(RenderViewConnection& connection, const int32_t image_id,
                                                           const ImageDescription& image_desc);

    OPENDCC_RENDER_VIEW_API int32_t render_view_write_region(RenderViewConnection& connection, const int32_t image_id, const ROI& region,
                                                             size_t pixel_size, const unsigned char* data);
}

OPENDCC_NAMESPACE_CLOSE
