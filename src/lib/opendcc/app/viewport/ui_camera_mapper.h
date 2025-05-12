/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "viewport_camera_mapper.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API UICameraMapper : public ViewportCameraMapper
{
public:
    UICameraMapper();
    void push(const PXR_NS::GfCamera& camera, PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) override;
    PXR_NS::GfCamera pull(PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) override;
    void set_path(const PXR_NS::SdfPath& path) override;
    PXR_NS::SdfPath get_path() override;
    bool is_camera_prim() const override;
    bool is_read_only() const override;
    bool is_valid() const override;

private:
    PXR_NS::GfCamera m_camera;
};

OPENDCC_NAMESPACE_CLOSE
