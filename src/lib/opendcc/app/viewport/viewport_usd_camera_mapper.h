/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_camera_mapper.h"
#include <pxr/usd/usd/prim.h>
#include "opendcc/app/core/session.h"

OPENDCC_NAMESPACE_OPEN

class ViewportUsdCameraMapper : public ViewportCameraMapper
{
public:
    ViewportUsdCameraMapper(const PXR_NS::SdfPath& path = PXR_NS::SdfPath());
    ~ViewportUsdCameraMapper() override;

    virtual void push(const PXR_NS::GfCamera& camera, PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) override;
    virtual PXR_NS::GfCamera pull(PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) override;
    virtual void set_path(const PXR_NS::SdfPath& path) override;
    virtual PXR_NS::SdfPath get_path() override;
    virtual bool is_camera_prim() const override;
    virtual bool is_read_only() const override;
    virtual bool is_valid() const override;

private:
    PXR_NS::UsdPrim m_prim;
    Session::StageChangedCallbackHandle m_follow_camera_changed_cid;
};

OPENDCC_NAMESPACE_CLOSE
