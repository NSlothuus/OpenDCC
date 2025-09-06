/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#if defined(__linux__)
#define OPENDCC_OS_LINUX
#elif defined(_WIN32) || defined(_WIN64)
#define OPENDCC_OS_WINDOWS
#elif defined(__APPLE__)
#define OPENDCC_OS_MAC
#endif

#if defined(__GNUC__)
#define OPENDCC_COMPILER_GCC
#define OPENDCC_COMPILER_GCC_MAJOR __GNUC__
#define OPENDCC_COMPILER_GCC_MINOR __GNUC_MINOR__
#define OPENDCC_COMPILER_GCC_PATCHLEVEL __GNUC_PATCHLEVEL__
#elif defined(_MSC_VER)
#define OPENDCC_COMPILER_MSVC
#define OPENDCC_COMPILER_MSVC_VERSION _MSC_VER
#endif

#if defined(__GNUC__) || defined(__clang__)
#define OPENDCC_WEAK_LINKAGE __attribute__((weak))
#else
#define OPENDCC_WEAK_LINKAGE __declspec(selectany)
#endif
