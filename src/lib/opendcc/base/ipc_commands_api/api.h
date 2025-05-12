/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <opendcc/base/export.h>

#ifdef IPC_COMMANDS_API_EXPORT
#define IPC_COMMANDS_API OPENDCC_API_EXPORT
#else
#define IPC_COMMANDS_API OPENDCC_API_IMPORT
#endif
