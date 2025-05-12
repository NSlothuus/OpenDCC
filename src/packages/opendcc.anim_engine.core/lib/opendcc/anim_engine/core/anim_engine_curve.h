/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/anim_engine/core/api.h"
#include "opendcc/anim_engine/curve/curve.h"
#include "pxr/usd/usd/attribute.h"

OPENDCC_NAMESPACE_OPEN

class StageListener;

class ANIM_ENGINE_API AnimEngineCurve : public AnimCurve
{
public:
    static const uint32_t undefined_id;

    AnimEngineCurve(PXR_NS::UsdAttribute attribute, uint32_t component_idx = undefined_id,
                    PXR_NS::SdfLayerHandle current_layer = PXR_NS::SdfLayerHandle());

    void save_to_attribute_metadata(StageListener& stage_listener, bool save_on_current_layer) const;

    void remove_from_metadata(StageListener& stage_listener) const;

    PXR_NS::UsdAttribute attribute() const { return m_attribute; }
    uint32_t component_idx() const { return m_component_idx; }

    static std::vector<AnimEngineCurve> create_from_metadata(PXR_NS::UsdAttribute& attr);

private:
    PXR_NS::SdfLayerHandle m_current_layer;
    PXR_NS::UsdAttribute m_attribute;
    uint32_t m_component_idx;
};

typedef std::shared_ptr<AnimEngineCurve> AnimEngineCurvePtr;
typedef std::shared_ptr<const AnimEngineCurve> AnimEngineCurveCPtr;

OPENDCC_NAMESPACE_CLOSE
