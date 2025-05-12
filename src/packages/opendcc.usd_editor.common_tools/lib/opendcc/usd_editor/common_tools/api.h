/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>
#ifdef OPENDCC_USD_EDITOR_COMMON_TOOLS_EXPORT
#define OPENDCC_USD_EDITOR_COMMON_TOOLS_API OPENDCC_API_EXPORT
#else
#define OPENDCC_USD_EDITOR_COMMON_TOOLS_API OPENDCC_API_IMPORT
#endif
