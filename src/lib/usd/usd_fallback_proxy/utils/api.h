/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <opendcc/base/export.h>

#ifdef FALLBACK_PROXY_UTILS_EXPORT
#define FALLBACK_PROXY_UTILS_API OPENDCC_API_EXPORT
#else
#define FALLBACK_PROXY_UTILS_API OPENDCC_API_IMPORT
#endif
