/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/utils/api.h"
#include <string>

OPENDCC_NAMESPACE_OPEN

UTILS_API void* dl_open(const std::string& filename, int flags);

UTILS_API int dl_close(void* handle);

UTILS_API void* dl_sym(void* handle, const char* name);

UTILS_API std::string dl_error_str();

UTILS_API int dl_error();

UTILS_API void* get_dl_handle(const std::string& library_name);

OPENDCC_NAMESPACE_CLOSE
