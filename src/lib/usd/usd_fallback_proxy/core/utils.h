/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "usd/usd_fallback_proxy/core/api.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/sdr/shaderProperty.h>

OPENDCC_NAMESPACE_OPEN

FALLBACK_PROXY_API PXR_NS::TfToken resolve_typename(const PXR_NS::SdrShaderPropertyConstPtr& prop);

OPENDCC_NAMESPACE_CLOSE
