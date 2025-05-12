// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/usd_ipc_serialization/usd_ipc_utils.h"
#include "opendcc/usd/usd_ipc_serialization/usd_edits.h"
#include <zmq.h>
#include <iostream>
#include <pxr/base/arch/threads.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

#ifdef OPENDCC_OS_WINDOWS
#define ZMQ_ERRNO zmq_errno()
#else
#define ZMQ_ERRNO errno
#endif

namespace usd_ipc_utils
{
    int32_t send_usd_edit(void* socket, const uint64_t& context_id, const UsdEditBase* edit)
    {
        zmq_msg_t zmsg;

        const auto buffer = edit->write();
        CHECK_ZMQ_ERROR_AND_RETURN_IT(zmq_msg_init_size(&zmsg, (buffer.size() + sizeof(uint64_t)) * sizeof(char)));

        auto msg_data = (char*)zmq_msg_data(&zmsg);
        memcpy(msg_data, &context_id, sizeof(context_id));
        memcpy(msg_data + sizeof(context_id), buffer.data(), buffer.size() * sizeof(char));

        CHECK_ZMQ_ERROR_AND_RETURN_IT(zmq_msg_send(&zmsg, socket, 0));
        CHECK_ZMQ_ERROR_AND_RETURN_IT(zmq_msg_close(&zmsg));
        return 0;
    }

    std::unique_ptr<UsdEditBase> receive_usd_edit(void* socket, uint64_t* context_id)
    {
        zmq_msg_t msg;
        CHECK_ZMQ_ERROR_AND_RETURN_VAL(zmq_msg_init(&msg), nullptr);
        CHECK_ZMQ_ERROR_AND_RETURN_VAL(zmq_msg_recv(&msg, socket, 0), nullptr);

        size_t buffer_size = zmq_msg_size(&msg);
        auto recv_data = (char*)zmq_msg_data(&msg);

        std::vector<char> buffer;
        constexpr auto context_id_size = sizeof(uint64_t);
        const auto edit_size = (buffer_size - context_id_size) / sizeof(char);
        buffer.resize(edit_size);

        if (context_id)
        {
            *context_id = *(reinterpret_cast<uint64_t*>(recv_data));
        }
        memcpy(buffer.data(), recv_data + context_id_size, edit_size);

        return UsdEditBase::read(buffer);
    }

    void send_response_code(void* socket, int32_t response_code)
    {
        zmq_msg_t msg_response;
        CHECK_ZMQ_ERROR_AND_RETURN(zmq_msg_init_size(&msg_response, sizeof(int32_t)));

        void* msg_data = zmq_msg_data(&msg_response);
        memcpy(msg_data, &response_code, sizeof(int32_t));

        CHECK_ZMQ_ERROR_AND_RETURN(zmq_msg_send(&msg_response, socket, 0));
        zmq_msg_close(&msg_response);
    }

    int32_t receive_response_code(void* socket)
    {
        zmq_msg_t zmsg;
        int32_t response_code = -1;

        CHECK_ZMQ_ERROR_AND_RETURN_VAL(zmq_msg_init(&zmsg), response_code);
        CHECK_ZMQ_ERROR_AND_RETURN_VAL(zmq_msg_recv(&zmsg, socket, 0), response_code);

        void* buffer = nullptr;
        buffer = zmq_msg_data(&zmsg);
        memcpy(&response_code, buffer, sizeof(int32_t));
        CHECK_ZMQ_ERROR_AND_RETURN_VAL(zmq_msg_close(&zmsg), response_code);
        return response_code;
    }

    void print_pretty_error(const char* function, size_t line, const char* file)
    {
        const auto error_code = ZMQ_ERRNO;
        if (error_code == 0 || error_code == ETERM)
            return;

        std::cerr << "ZMQ_ERROR " << error_code << (ArchIsMainThread() ? "" : " (secondary thread)") << ": in " << function << " at line " << line
                  << " of " << file << " -- " << zmq_strerror(error_code) << '\n';
    }

};
OPENDCC_NAMESPACE_CLOSE
