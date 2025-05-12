/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <opendcc/base/export.h>
#ifdef OPENDCC_LOGGER_PANEL_EXPORT
#define OPENDCC_LOGGER_PANEL_API OPENDCC_API_EXPORT
#else
#define OPENDCC_LOGGER_PANEL_API OPENDCC_API_IMPORT
#endif
