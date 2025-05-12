/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <opendcc/base/export.h>

#ifdef COMMANDS_API_EXPORT
#define COMMANDS_API OPENDCC_API_EXPORT
#else
#define COMMANDS_API OPENDCC_API_IMPORT
#endif
