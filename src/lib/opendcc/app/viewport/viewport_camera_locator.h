/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/viewport/viewport_locator_data.h"
#include <pxr/base/gf/frustum.h>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API CameraLocatorRenderData : public LocatorRenderData
{
public:
    CameraLocatorRenderData();

    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& prim) override;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const override;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const override;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const override;
    virtual const PXR_NS::GfRange3d& bbox() const override;
    virtual const PXR_NS::TfToken& topology() const override;

private:
    void update_points();

    PXR_NS::GfFrustum::ProjectionType m_proj_type = PXR_NS::GfFrustum::Perspective;
    std::vector<PXR_NS::GfVec3d> m_near_far_corners;

    PXR_NS::VtArray<PXR_NS::GfVec3f> m_points;
    PXR_NS::GfRange3d m_bbox;
};

OPENDCC_NAMESPACE_CLOSE