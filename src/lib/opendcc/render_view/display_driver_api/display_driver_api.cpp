// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <zmq.h>
#include <vector>
#include "opendcc/render_view/display_driver_api/display_driver_api.h"
#include <memory.h>

OPENDCC_NAMESPACE_OPEN

namespace display_driver_api
{

    template <class T>
    void save_to_buffer(const T& obj, std::vector<char>& buffer)
    {

        size_t offset = buffer.size();
        size_t new_size = sizeof(T) + offset;
        buffer.resize(new_size / sizeof(char));
        memcpy(buffer.data() + offset, &obj, sizeof(T));
    }

    template <>
    void save_to_buffer(const std::string& obj, std::vector<char>& buffer)
    {

        save_to_buffer<uint32_t>(obj.size(), buffer);
        for (uint32_t i = 0; i < obj.size(); i++)
        {
            save_to_buffer<char>(obj[i], buffer);
        }
    }

    template <class T>
    void save_to_buffer(const std::vector<T>& obj, std::vector<char>& buffer)
    {

        save_to_buffer<uint32_t>(obj.size(), buffer);
        size_t offset = buffer.size();
        size_t new_size = obj.size() * sizeof(T) + offset;
        buffer.resize(new_size / sizeof(char));

        memcpy(buffer.data() + offset, obj.data(), obj.size() * sizeof(T));
    }

    template <>
    void save_to_buffer(const std::map<std::string, std::string>& obj, std::vector<char>& buffer)
    {

        save_to_buffer<uint32_t>(obj.size(), buffer);

        for (auto& it : obj)
        {
            save_to_buffer(it.first, buffer);
            save_to_buffer(it.second, buffer);
        }
    }

    template <class T>
    void load_from_buffer(T& obj, const std::vector<char>& buffer, size_t& offset)
    {
        const char* ptr = (buffer.data() + offset);
        obj = *((T*)ptr);
        offset += sizeof(T);
    }

    template <>
    void load_from_buffer(std::string& obj, const std::vector<char>& buffer, size_t& offset)
    {

        uint32_t string_size;

        load_from_buffer<uint32_t>(string_size, buffer, offset);

        obj.resize(string_size);

        for (uint32_t i = 0; i < string_size; i++)
        {
            load_from_buffer<char>(obj[i], buffer, offset);
        }
    }

    template <class T>
    void load_from_buffer(std::vector<T>& obj, const std::vector<char>& buffer, size_t& offset)
    {
        uint32_t vector_size;

        load_from_buffer<uint32_t>(vector_size, buffer, offset);

        obj.resize(vector_size);

        memcpy(obj.data(), buffer.data() + offset, sizeof(T) * vector_size);
        offset += sizeof(T) * vector_size;
    }

    template <>
    void load_from_buffer(std::map<std::string, std::string>& obj, const std::vector<char>& buffer, size_t& offset)
    {
        uint32_t vector_size;

        load_from_buffer<uint32_t>(vector_size, buffer, offset);

        for (uint32_t i = 0; i < vector_size; i++)
        {
            std::string first, second;
            load_from_buffer(first, buffer, offset);
            load_from_buffer(second, buffer, offset);

            obj.insert(std::make_pair(first, second));
        }
    }

    template <>
    void save_to_buffer(const ROI& obj, std::vector<char>& buffer)
    {
        save_to_buffer(obj.xstart, buffer);
        save_to_buffer(obj.xend, buffer);

        save_to_buffer(obj.ystart, buffer);
        save_to_buffer(obj.yend, buffer);
    }

    template <>
    void save_to_buffer(const ImageDescription& obj, std::vector<char>& buffer)
    {
        save_to_buffer(obj.parent_image_id, buffer);
        save_to_buffer(obj.image_name, buffer);
        save_to_buffer(obj.image_data_type, buffer);
        save_to_buffer(obj.num_channels, buffer);
        save_to_buffer(obj.width, buffer);
        save_to_buffer(obj.height, buffer);
        save_to_buffer(obj.extra_attributes, buffer);
    }

    void save_msg_to_buffer(const Message& msg, std::vector<char>& buffer)
    {

        save_to_buffer(msg.type, buffer);
        save_to_buffer(msg.image_id, buffer);
        save_to_buffer(msg.image_desc, buffer);
        save_to_buffer(msg.region, buffer);
        save_to_buffer(msg.bucket_data, buffer);
    }

    template <>
    void load_from_buffer(ROI& obj, const std::vector<char>& buffer, size_t& offset)
    {
        load_from_buffer(obj.xstart, buffer, offset);
        load_from_buffer(obj.xend, buffer, offset);

        load_from_buffer(obj.ystart, buffer, offset);
        load_from_buffer(obj.yend, buffer, offset);
    }

    template <>
    void load_from_buffer(ImageDescription& obj, const std::vector<char>& buffer, size_t& offset)
    {
        load_from_buffer(obj.parent_image_id, buffer, offset);
        load_from_buffer(obj.image_name, buffer, offset);
        load_from_buffer(obj.image_data_type, buffer, offset);
        load_from_buffer(obj.num_channels, buffer, offset);
        load_from_buffer(obj.width, buffer, offset);
        load_from_buffer(obj.height, buffer, offset);
        load_from_buffer(obj.extra_attributes, buffer, offset);
    }

    void load_msg_from_buffer(Message& msg, const std::vector<char>& buffer)
    {
        size_t offset = 0;

        load_from_buffer(msg.type, buffer, offset);
        load_from_buffer(msg.image_id, buffer, offset);
        load_from_buffer(msg.image_desc, buffer, offset);

        load_from_buffer(msg.region, buffer, offset);
        load_from_buffer(msg.bucket_data, buffer, offset);
    }

    int32_t render_view_open_image(RenderViewConnection& connection, const int32_t image_id, const ImageDescription& image_desc)
    {

        Message msg;
        msg.type = Message::Type::OpenImage;
        msg.image_id = image_id;
        msg.image_desc = image_desc;

        connection.send_msg(msg);
        return connection.recv_msg();
    }

    int32_t render_view_write_region(RenderViewConnection& connection, const int32_t image_id, const ROI& region, size_t pixel_size,
                                     const unsigned char* data)
    {
        Message msg;
        msg.type = Message::Type::WriteRegion;
        msg.image_id = image_id;
        msg.region = region;

        size_t data_size = (region.xend - region.xstart) * (region.yend - region.ystart) * pixel_size;
        msg.bucket_data.resize(data_size / sizeof(unsigned char));

        memcpy(msg.bucket_data.data(), data, data_size);

        connection.send_msg(msg);
        return connection.recv_msg();
    }

    static void* shared_zmq_context = nullptr;
    static int shared_zmq_context_ref = 0;

    class RenderViewConnectionImpl
    {
        void* m_context = nullptr;
        void* m_socket = nullptr;

    public:
        void init()
        {
            if (!shared_zmq_context)
            {
                shared_zmq_context = zmq_ctx_new();
            }

            m_context = shared_zmq_context;
            shared_zmq_context_ref++;
            m_socket = zmq_socket(m_context, ZMQ_REQ);
            const int timeout = 10000;
            zmq_setsockopt(m_socket, ZMQ_RCVTIMEO, &timeout, sizeof(int));
            // data->image_handle = -1;

            // TODO get port from driver parameters;
            zmq_connect(m_socket, "tcp://127.0.0.1:5556");
        }
        void destroy()
        {
            zmq_close(m_socket);
            m_socket = nullptr;
            if (shared_zmq_context_ref == 0)
            {
                zmq_ctx_destroy(shared_zmq_context);
                shared_zmq_context = nullptr;
            }
            else
            {
                shared_zmq_context_ref--;
            }
            m_context = nullptr;
        }
        void send_msg(Message& msg)
        {
            zmq_msg_t zmsg;

            std::vector<char> buffer;

            save_msg_to_buffer(msg, buffer);
            zmq_msg_init_size(&zmsg, buffer.size() * sizeof(char));

            void* msg_data = zmq_msg_data(&zmsg);
            memcpy(msg_data, buffer.data(), buffer.size() * sizeof(char));

            int rc = zmq_msg_send(&zmsg, m_socket, 0);

            zmq_msg_close(&zmsg);
        }
        int32_t recv_msg()
        {
            zmq_msg_t zmsg;
            int32_t response_code = -1;

            zmq_msg_init(&zmsg);
            int rc = zmq_msg_recv(&zmsg, m_socket, 0);

            void* buff = nullptr;
            buff = zmq_msg_data(&zmsg);
            memcpy(&response_code, buff, sizeof(int32_t));
            zmq_msg_close(&zmsg);

            return response_code;
        }
    };

    RenderViewConnection::RenderViewConnection()
        : m_impl { std::make_unique<RenderViewConnectionImpl>() }
    {

        m_impl->init();
    }

    RenderViewConnection::~RenderViewConnection()
    {
        m_impl->destroy();
    }

    void RenderViewConnection::send_msg(Message& msg)
    {
        m_impl->send_msg(msg);
    }

    int32_t RenderViewConnection::recv_msg()
    {
        return m_impl->recv_msg();
    }

}

OPENDCC_NAMESPACE_CLOSE
