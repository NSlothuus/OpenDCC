/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>
#ifdef EXPRESSION_ENGINE_EXPORT
#define EXPRESSION_ENGINE_API OPENDCC_API_EXPORT
#else
#define EXPRESSION_ENGINE_API OPENDCC_API_IMPORT
#endif
