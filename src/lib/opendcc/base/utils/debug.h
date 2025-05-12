/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/utils/api.h"

#ifndef OPENDCC_DEBUG_BUILD
#if defined(_DEBUG) || defined(DEBUG)
#define OPENDCC_DEBUG_BUILD
#endif
#endif

OPENDCC_NAMESPACE_OPEN

// Implemented only for Windows
UTILS_API bool is_debugged();
UTILS_API void trap_debugger();

OPENDCC_NAMESPACE_CLOSE
