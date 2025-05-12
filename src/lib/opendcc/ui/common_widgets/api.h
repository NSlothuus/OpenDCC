/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/base/export.h"

#ifdef OPENDCC_UI_COMMON_WIDGETS_EXPORT
#define OPENDCC_UI_COMMON_WIDGETS_API OPENDCC_API_EXPORT
#else
#define OPENDCC_UI_COMMON_WIDGETS_API OPENDCC_API_IMPORT
#endif
