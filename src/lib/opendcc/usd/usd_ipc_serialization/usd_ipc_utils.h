/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/base/defines.h"
#include <memory>
#include "opendcc/usd/usd_ipc_serialization/api.h"
#include <string>

OPENDCC_NAMESPACE_OPEN
class UsdEditBase;

namespace usd_ipc_utils
{
    USD_IPC_SERIALIZATION_API int32_t send_usd_edit(void* socket, const uint64_t& context_id, const UsdEditBase* edit);
    USD_IPC_SERIALIZATION_API std::unique_ptr<UsdEditBase> receive_usd_edit(void* socket, uint64_t* context_id);
    USD_IPC_SERIALIZATION_API void send_response_code(void* socket, int32_t response);
    USD_IPC_SERIALIZATION_API int32_t receive_response_code(void* socket);
    USD_IPC_SERIALIZATION_API void print_pretty_error(const char* function, size_t line, const char* file);
};

#define CHECK_ZMQ_ERROR_AND_RETURN(action)                                       \
    do                                                                           \
    {                                                                            \
        if ((action) == -1)                                                      \
        {                                                                        \
            usd_ipc_utils::print_pretty_error(__FUNCTION__, __LINE__, __FILE__); \
            return;                                                              \
        }                                                                        \
    } while (false)

#define CHECK_ZMQ_ERROR_AND_RETURN_IT(action)                                    \
    do                                                                           \
    {                                                                            \
        const int rc = (action);                                                 \
        if (rc == -1)                                                            \
        {                                                                        \
            usd_ipc_utils::print_pretty_error(__FUNCTION__, __LINE__, __FILE__); \
            return rc;                                                           \
        }                                                                        \
    } while (false)

#define CHECK_ZMQ_ERROR_AND_RETURN_VAL(action, val)                              \
    do                                                                           \
    {                                                                            \
        if ((action) == -1)                                                      \
        {                                                                        \
            usd_ipc_utils::print_pretty_error(__FUNCTION__, __LINE__, __FILE__); \
            return val;                                                          \
        }                                                                        \
    } while (false)

OPENDCC_NAMESPACE_CLOSE
