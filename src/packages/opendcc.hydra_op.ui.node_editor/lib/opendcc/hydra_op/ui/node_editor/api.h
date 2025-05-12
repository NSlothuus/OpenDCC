/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>
#ifdef OPENDCC_HYDRA_OP_NODE_EDITOR_EXPORT
#define OPENDCC_HYDRA_OP_NODE_EDITOR_API OPENDCC_API_EXPORT
#else
#define OPENDCC_HYDRA_OP_NODE_EDITOR_API OPENDCC_API_IMPORT
#endif