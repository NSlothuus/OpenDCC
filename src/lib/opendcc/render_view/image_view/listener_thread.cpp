// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <zmq.h>
#include <vector>
#include "opendcc/render_view/display_driver_api/display_driver_api.h"

#include "app.h"

#include "listener_thread.hpp"

OPENDCC_NAMESPACE_OPEN

void RenderViewListenerThread::run()
{
    void* listener = zmq_socket(m_zmq_ctx, ZMQ_REP);

    zmq_bind(listener, "tcp://127.0.0.1:5556");

    while (true)
    {
        if (isInterruptionRequested())
            break;
        zmq_msg_t msg;
        zmq_msg_init(&msg);
        int rc = zmq_msg_recv(&msg, listener, 0);

        if (rc != -1)
        {
            size_t buff_size = zmq_msg_size(&msg);

            void* buff = nullptr;
            buff = zmq_msg_data(&msg);

            std::vector<char> buffer;
            buffer.resize(buff_size / sizeof(char));

            memcpy(buffer.data(), buff, buff_size);
            display_driver_api::Message message;

            display_driver_api::load_msg_from_buffer(message, buffer);

            zmq_msg_t msg_response;
            int32_t response_code = -1;
            switch (message.type)
            {
            case display_driver_api::Message::Type::OpenImage:
            {
                RenderViewMainWindow::ImageDataType data_type = RenderViewMainWindow::ImageDataType::Float;
                switch (message.image_desc.image_data_type)
                {
                case display_driver_api::ImageType::Byte:
                    data_type = RenderViewMainWindow::ImageDataType::Byte;
                    break;
                case display_driver_api::ImageType::UInt:
                    data_type = RenderViewMainWindow::ImageDataType::UInt;
                    break;
                case display_driver_api::ImageType::Int:
                    data_type = RenderViewMainWindow::ImageDataType::Int;
                    break;
                case display_driver_api::ImageType::Float:
                    data_type = RenderViewMainWindow::ImageDataType::Float;
                    break;
                case display_driver_api::ImageType::HalfFloat:
                    data_type = RenderViewMainWindow::ImageDataType::HalfFloat;
                    break;
                default:
                    break;
                }
                RenderViewMainWindow::ImageDesc image_desc;

                image_desc.image_type = data_type;
                image_desc.num_channels = message.image_desc.num_channels;
                image_desc.width = message.image_desc.width;
                image_desc.height = message.image_desc.height;
                image_desc.image_name = message.image_desc.image_name;
                image_desc.parent_image_id = message.image_desc.parent_image_id;
                image_desc.extra_attributes = message.image_desc.extra_attributes;

                response_code = m_app->create_image(message.image_id, image_desc);
                emit new_image();
                break;
            }
            case display_driver_api::Message::Type::WriteRegion:
            {
                RenderViewMainWindow::ImageROI region;

                region.xstart = message.region.xstart;
                region.xend = message.region.xend;
                region.ystart = message.region.ystart;
                region.yend = message.region.yend;

                m_app->update_image((int)message.image_id, region, message.bucket_data);
                response_code = 0;
                break;
            }
            default:
                break;
            }
            // send response
            zmq_msg_init_size(&msg_response, sizeof(int32_t));

            void* msg_data = zmq_msg_data(&msg_response);
            memcpy(msg_data, &response_code, sizeof(int32_t));

            rc = zmq_msg_send(&msg_response, listener, 0);

            zmq_msg_close(&msg_response);
        }
        zmq_msg_close(&msg);
    }
    zmq_close(listener);
}

OPENDCC_NAMESPACE_CLOSE
