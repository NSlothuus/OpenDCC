/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>
#ifdef ANIM_ENGINE_EXPORT
#define ANIM_ENGINE_API OPENDCC_API_EXPORT
#else
#define ANIM_ENGINE_API OPENDCC_API_IMPORT
#endif