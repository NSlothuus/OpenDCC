/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "pxr/usd/usd/attribute.h"
#include "opendcc/anim_engine/core/api.h"
#include "opendcc/anim_engine/core/engine.h"

OPENDCC_NAMESPACE_OPEN

enum class AttributeClass
{
    Scalar,
    Vector, // .xyzw
    Color,
    Other
};

bool ANIM_ENGINE_API is_attribute_may_animated(const PXR_NS::UsdAttribute& usd_attr);
AttributeClass ANIM_ENGINE_API attribute_class(const PXR_NS::UsdAttribute& usd_attr);
uint32_t ANIM_ENGINE_API num_components_in_attribute(const PXR_NS::UsdAttribute& usd_attr);
bool ANIM_ENGINE_API set_usd_attribute_component(PXR_NS::UsdAttribute usd_attr, uint32_t component, double value);
bool ANIM_ENGINE_API set_usd_attribute_component(PXR_NS::UsdAttribute usd_attr, std::vector<uint32_t> components, std::vector<double> values,
                                                 PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default());
bool ANIM_ENGINE_API get_usd_attribute_component(const PXR_NS::UsdAttribute& usd_attr, uint32_t component, double& value,
                                                 PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default());
AnimEngine::CurveIdToKeysIdsMap keyframes_to_keyIds(const AnimEngine::CurveIdToKeyframesMap&);

OPENDCC_NAMESPACE_CLOSE
