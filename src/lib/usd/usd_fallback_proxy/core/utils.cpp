// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <usd_fallback_proxy/core/utils.h>

OPENDCC_NAMESPACE_OPEN

PXR_NS::TfToken resolve_typename(const PXR_NS::SdrShaderPropertyConstPtr& prop)
{
    // For now just assume that first element of pair / GetSdfType will always contain type defined in SdfValueTypeNames
    const auto& sdf_type = prop->GetTypeAsSdfType();
#if PXR_VERSION >= 2411
    return sdf_type.GetSdfType().GetAsToken();
#else
    return sdf_type.first.GetAsToken();
#endif
}

OPENDCC_NAMESPACE_CLOSE
