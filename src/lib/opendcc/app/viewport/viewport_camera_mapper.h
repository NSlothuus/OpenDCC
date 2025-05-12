/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/base/gf/camera.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/timeCode.h>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API ViewportCameraMapper
{
public:
    ViewportCameraMapper() = default;
    virtual ~ViewportCameraMapper() = default;

    virtual void push(const PXR_NS::GfCamera& camera, PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) = 0;
    virtual PXR_NS::GfCamera pull(PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) = 0;
    virtual void set_path(const PXR_NS::SdfPath& path) = 0;
    virtual PXR_NS::SdfPath get_path() = 0;
    virtual bool is_camera_prim() const = 0;
    virtual bool is_read_only() const = 0;
    virtual bool is_valid() const = 0;
    virtual void set_prim_changed_callback(std::function<void()> callback) { m_prim_changed_callback = callback; }

protected:
    std::function<void()> m_prim_changed_callback;
    PXR_NS::SdfPath m_path;
};

using ViewportCameraMapperPtr = std::shared_ptr<ViewportCameraMapper>;

OPENDCC_NAMESPACE_CLOSE
