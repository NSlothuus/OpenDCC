/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <opendcc/base/export.h>

#ifdef ARNOLD_UTILS_EXPORT
#define ARNOLD_UTILS_API OPENDCC_API_EXPORT
#else
#define ARNOLD_UTILS_API OPENDCC_API_IMPORT
#endif