/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/utils/api.h"
#include <string>

OPENDCC_NAMESPACE_OPEN

UTILS_API std::string get_env(const std::string_view& name);
UTILS_API bool set_env(const std::string_view& name, const std::string_view& value);

OPENDCC_NAMESPACE_CLOSE
