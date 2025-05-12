// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/bezier_tool/utils.h"

#include <pxr/base/gf/vec4d.h>
#include <pxr/base/gf/line.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

GfVec2f clip_from_screen(const GfVec2f& screen, const int w, const int h)
{
    return { 2.0f * screen[0] / w - 1, 1 - 2.0f * screen[1] / h };
}

PXR_NS::GfVec2f screen_from_clip(const PXR_NS::GfVec2f& clip, const int w, const int h)
{
    return { 0.5f * w * (clip[0] + 1), 0.5f * h * (1 - clip[1]) };
}

GfMatrix4d compute_view_projection(const ViewportViewPtr& viewport_view)
{
    const auto w = viewport_view->get_viewport_dimensions().width;
    const auto h = viewport_view->get_viewport_dimensions().height;

    auto frustum = viewport_view->get_camera().GetFrustum();
    CameraUtilConformWindow(&frustum, PXR_NS::CameraUtilConformWindowPolicy::CameraUtilFit, h != 0.0 ? w / static_cast<double>(h) : 1.0);

    return frustum.ComputeViewMatrix() * frustum.ComputeProjectionMatrix();
}

bool lie_on_one_line(const PXR_NS::GfVec3f& f, const PXR_NS::GfVec3f& s, const PXR_NS::GfVec3f& t)
{
    bool equal = GfIsClose(f, s, epsilon) || GfIsClose(s, t, epsilon) || GfIsClose(f, t, epsilon);
    if (equal)
    {
        return true;
    }

    GfLine line(f, f - s);
    const auto closest = GfVec3f(line.FindClosestPoint(GfVec3d(t)));
    return GfIsClose(closest, t, epsilon);
}

OPENDCC_NAMESPACE_CLOSE
