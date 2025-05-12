/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>
#ifdef BULLET_PHYSICS_EXPORT
#define BULLET_PHYSICS_API OPENDCC_API_EXPORT
#else
#define BULLET_PHYSICS_API OPENDCC_API_IMPORT
#endif