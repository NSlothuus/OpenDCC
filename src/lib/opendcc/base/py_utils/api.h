/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>
#ifdef OPENDCC_PY_UTILS_EXPORT
#define OPENDCC_PY_UTILS_API OPENDCC_API_EXPORT
#else
#define OPENDCC_PY_UTILS_API OPENDCC_API_IMPORT
#endif
