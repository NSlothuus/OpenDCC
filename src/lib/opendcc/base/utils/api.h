/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <opendcc/base/export.h>

#ifdef UTILS_API_EXPORT
#define UTILS_API OPENDCC_API_EXPORT
#else
#define UTILS_API OPENDCC_API_IMPORT
#endif
