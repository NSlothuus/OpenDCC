// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "utils.h"

#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/camera.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>

float line_plane_intersection(const PXR_NS::GfVec3f& line_direction, const PXR_NS::GfVec3f& point_on_line, const PXR_NS::GfVec3f& plane_normal,
                              const PXR_NS::GfVec3f& point_on_plane)
{
    const float l_dot_n = line_direction * plane_normal;
    if (std::abs(l_dot_n) < 1e-5)
    {
        return FLT_MAX;
    }
    else
    {
        return ((point_on_plane - point_on_line) * plane_normal) / l_dot_n;
    }
}

void solve_ray_info(const OPENDCC_NAMESPACE::ViewportMouseEvent& mouse_event, const OPENDCC_NAMESPACE::ViewportViewPtr& viewport_view,
                    PXR_NS::GfVec3f& start, PXR_NS::GfVec3f& direction)
{
    const int h = viewport_view->get_viewport_dimensions().height;
    const int w = viewport_view->get_viewport_dimensions().width;

    const PXR_NS::GfVec4d viewport_resolution(0, 0, w, h);
    PXR_NS::GfFrustum frustum = viewport_view->get_camera().GetFrustum();

    CameraUtilConformWindow(&frustum, PXR_NS::CameraUtilConformWindowPolicy::CameraUtilFit,
                            viewport_resolution[3] != 0.0 ? viewport_resolution[2] / viewport_resolution[3] : 1.0);

    const float screen_x = 2.0f * mouse_event.x() / w - 1;
    const float screen_y = 1 - 2.0f * mouse_event.y() / h;

    const PXR_NS::GfVec2d pick_point(screen_x, screen_y);
    const PXR_NS::GfRay pick_ray = frustum.ComputePickRay(pick_point);

    start = PXR_NS::GfVec3f(pick_ray.GetStartPoint());
    direction = PXR_NS::GfVec3f(pick_ray.GetDirection());
}
