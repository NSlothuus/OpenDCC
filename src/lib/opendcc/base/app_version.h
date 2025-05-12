/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/base/export.h"

#ifdef OPENDCC_APP_VERSION_EXPORT
#define OPENDCC_APP_VERSION_API OPENDCC_API_EXPORT
#else
#define OPENDCC_APP_VERSION_API OPENDCC_API_IMPORT
#endif

OPENDCC_NAMESPACE_OPEN

namespace platform
{
    /**
     * @brief Get the build date str from compiler using __DATE__ macro
     *
     * @return const char*
     */
    OPENDCC_APP_VERSION_API const char* get_build_date_str();
    /**
     * @brief Get the git commit hash of current build
     *
     * @return const char*
     */
    OPENDCC_APP_VERSION_API const char* get_git_commit_hash_str();
}

OPENDCC_NAMESPACE_CLOSE
