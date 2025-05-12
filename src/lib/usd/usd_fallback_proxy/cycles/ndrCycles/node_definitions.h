/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "usd_fallback_proxy/cycles/ndrCycles/api.h"
#include <pxr/usd/usd/stage.h>

OPENDCC_NAMESPACE_OPEN

NDR_CYCLES_API PXR_NS::UsdStageRefPtr get_node_definitions();

OPENDCC_NAMESPACE_CLOSE
