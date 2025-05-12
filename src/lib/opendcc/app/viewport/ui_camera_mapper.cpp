// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/ui_camera_mapper.h"

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN

UICameraMapper::UICameraMapper()
{
    const auto start_pos = GfMatrix4d().SetLookAt(GfVec3d(0.5, 0.5, 1), GfVec3d(0.5, 0.5, 0), GfVec3d::YAxis());
    m_camera = GfCamera(start_pos, GfCamera::Orthographic, GfCamera::DEFAULT_HORIZONTAL_APERTURE, GfCamera::DEFAULT_VERTICAL_APERTURE, 0.0f, 0.0f,
                        50.0f, GfRange1f(0.1f, 300.0f));
}

void UICameraMapper::push(const GfCamera& camera, UsdTimeCode time /*= UsdTimeCode::Default()*/)
{
    m_camera = camera;
}

bool UICameraMapper::is_valid() const
{
    return true;
}

bool UICameraMapper::is_read_only() const
{
    return false;
}

bool UICameraMapper::is_camera_prim() const
{
    return false;
}

SdfPath UICameraMapper::get_path()
{
    return SdfPath::EmptyPath();
}

void UICameraMapper::set_path(const SdfPath& path)
{
    // empty
}

GfCamera UICameraMapper::pull(UsdTimeCode time /*= UsdTimeCode::Default()*/)
{
    return m_camera;
}

OPENDCC_NAMESPACE_CLOSE
