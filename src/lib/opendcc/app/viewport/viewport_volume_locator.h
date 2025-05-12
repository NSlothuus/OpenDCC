/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/app/core/api.h"
#include "opendcc/app/viewport/viewport_locator_data.h"
#include "pxr/base/vt/array.h"
#include <pxr/base/gf/vec3f.h>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API VolumeLocatorRenderData : public LocatorRenderData
{
public:
    VolumeLocatorRenderData();

    virtual void update(const std::unordered_map<std::string, PXR_NS::VtValue>& data) override;
    virtual const PXR_NS::VtArray<int>& vertex_per_curve() const override;
    virtual const PXR_NS::VtArray<int>& vertex_indexes() const override;
    virtual const PXR_NS::VtVec3fArray& vertex_positions() const override;
    virtual const PXR_NS::GfRange3d& bbox() const override;
    virtual const PXR_NS::TfToken& topology() const override;

    virtual const bool as_mesh() const override;

    virtual const bool is_double_sided() const override;

private:
    void update_points();

    PXR_NS::VtVec3fArray m_points;
    PXR_NS::GfRange3d m_bbox;
};

OPENDCC_NAMESPACE_CLOSE