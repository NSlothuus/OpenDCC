/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>
#ifdef NDR_CYCLES_EXPORT
#define NDR_CYCLES_API OPENDCC_API_EXPORT
#else
#define NDR_CYCLES_API OPENDCC_API_IMPORT
#endif