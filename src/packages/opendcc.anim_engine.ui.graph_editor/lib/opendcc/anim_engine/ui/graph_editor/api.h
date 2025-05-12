/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>
#ifdef GRAPH_EDITOR_EXPORT
#define GRAPH_EDITOR_API OPENDCC_API_EXPORT
#else
#define GRAPH_EDITOR_API OPENDCC_API_IMPORT
#endif