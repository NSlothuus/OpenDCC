/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <opendcc/base/defines.h>

#ifdef OPENDCC_OS_WINDOWS
#define OPENDCC_API_EXPORT __declspec(dllexport)
#define OPENDCC_API_IMPORT __declspec(dllimport)
#define OPENDCC_API_HIDDEN
#elif defined(OPENDCC_OS_LINUX) || defined(OPENDCC_OS_MAC)
#define OPENDCC_API_EXPORT __attribute__((visibility("default")))
#define OPENDCC_API_IMPORT
#define OPENDCC_API_HIDDEN __attribute__((visibility("hidden")))
#else
#define OPENDCC_API_EXPORT
#define OPENDCC_API_IMPORT
#define OPENDCC_API_HIDDEN
#endif
