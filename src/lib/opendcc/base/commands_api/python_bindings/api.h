/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <opendcc/base/export.h>
#ifdef PY_COMMANDS_API_EXPORT
#define PY_COMMANDS_API OPENDCC_API_EXPORT
#else
#define PY_COMMANDS_API OPENDCC_API_IMPORT
#endif