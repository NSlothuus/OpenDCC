/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>

#ifdef USD_IPC_SERIALIZATION_EXPORT
#define USD_IPC_SERIALIZATION_API OPENDCC_API_EXPORT
#else
#define USD_IPC_SERIALIZATION_API OPENDCC_API_IMPORT
#endif
