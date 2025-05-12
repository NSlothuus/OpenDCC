// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/uv_editor/utils.h"

#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include "opendcc/app/core/application.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

//////////////////////////////////////////////////////////////////////////
// DrawScope
//////////////////////////////////////////////////////////////////////////

DrawScope::DrawScope(ViewportUiDrawManager* manager, const uint32_t id /* = 0 */)
    : m_manager(manager)
    , m_id(id)
{
    m_manager->begin_drawable(m_id);
}

DrawScope::~DrawScope()
{
    m_manager->end_drawable();
}
//////////////////////////////////////////////////////////////////////////
// draw
//////////////////////////////////////////////////////////////////////////

BaseDrawInfo::BaseDrawInfo()
{
    mvp.SetIdentity();
}

//////////////////////////////////////////////////////////////////////////
// draw_axis
//////////////////////////////////////////////////////////////////////////

void draw_axis(ViewportUiDrawManager* manager, const AxisInfo& info, uint32_t id)
{
    DrawScope scope(manager, id);

    manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
    manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLines);

    const auto begin = GfVec3f(info.begin[0], info.begin[1], 0);
    const auto end_2d = info.begin + info.direction * info.length;
    const auto end = GfVec3f(end_2d[0], end_2d[1], 0);

    manager->set_mvp_matrix(info.mvp);
    manager->set_color(info.color);
    manager->set_line_width(info.width);
    manager->line(begin, end);
}

//////////////////////////////////////////////////////////////////////////
// draw_circle
//////////////////////////////////////////////////////////////////////////

void draw_circle(ViewportUiDrawManager* manager, const CircleInfo& info, uint32_t id)
{
    DrawScope scope(manager, id);

    manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
    manager->set_mvp_matrix(info.mvp);
    manager->set_color(info.color);
    manager->set_line_width(info.width);
    manager->set_depth_priority(info.depth_priority);
    GfVec3f origin { info.origin[0], info.origin[1], 0.0f };

    std::vector<GfVec3f> points;
    for (int i = 0; i < 50; i++)
    {
        GfVec3f vt;
        vt = info.right * cos((2 * M_PI / 50) * i);
        vt += info.up * sin((2 * M_PI / 50) * i);
        vt += origin;
        points.push_back(vt);
    }
    manager->mesh(ViewportUiDrawManager::PrimitiveTypeLinesLoop, points);
}

//////////////////////////////////////////////////////////////////////////
// draw_pie
//////////////////////////////////////////////////////////////////////////

void draw_pie(ViewportUiDrawManager* manager, const PieInfo& info, uint32_t id)
{
    // draw start/end vectors
    {
        DrawScope outlined_scope(manager, id);
        manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLines);
        manager->set_color(info.color);
        manager->set_mvp_matrix(info.mvp);
        manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
        manager->line(info.origin, info.start);
        manager->line(info.origin, info.end);
        manager->set_line_width(info.width);
        manager->set_depth_priority(info.depth_priority);
    }

    // draw arc
    {
        DrawScope outlined_scope(manager, id);
        manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeTriangleFan);
        manager->set_mvp_matrix(info.mvp);
        manager->set_depth_priority(info.depth_priority);
        manager->set_paint_style(ViewportUiDrawManager::PaintStyle::STIPPLED);
        manager->set_color(info.color);
        manager->arc(info.origin, info.start, info.end, -info.view, info.radius, true);
    }

    // draw points
    {
        DrawScope outlined_scope(manager, id);
        const GfVec3f v1_screen_pos = info.mvp.Transform(info.start);
        const GfVec3f v2_screen_pos = info.mvp.Transform(info.end);
        const GfVec3f orig_screen_pos = info.mvp.Transform(info.origin);
        manager->set_color(info.color);
        manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
        manager->set_depth_priority(info.depth_priority);
        manager->set_point_size(info.point_size);
        manager->mesh(ViewportUiDrawManager::PrimitiveType::PrimitiveTypePoints, { v1_screen_pos, v2_screen_pos, orig_screen_pos });
    }
}

//////////////////////////////////////////////////////////////////////////
// draw_quad
//////////////////////////////////////////////////////////////////////////

void draw_quad(ViewportUiDrawManager* manager, const QuadInfo& info, uint32_t id, float x_offset, float y_offset)
{
    const auto w = std::abs(info.max[0] - info.min[0]);
    const auto h = std::abs(info.max[1] - info.min[1]);

    std::vector<GfVec3f> vertices;

    vertices.push_back(GfVec3f(info.max[0] + x_offset, info.max[1] + y_offset, 0.0f));
    vertices.push_back(GfVec3f(info.max[0] + x_offset, info.max[1] + y_offset - h, 0.0f));
    vertices.push_back(GfVec3f(info.min[0] + x_offset, info.min[1] + y_offset, 0.0f));
    vertices.push_back(GfVec3f(info.min[0] + x_offset, info.min[1] + y_offset + h, 0.0f));

    {
        DrawScope quad_scope(manager, id);
        manager->set_mvp_matrix(info.mvp);
        manager->set_color(info.color);
        manager->set_depth_priority(info.depth_priority);
        manager->mesh(ViewportUiDrawManager::PrimitiveTypeTriangleFan, vertices);
    }

    if (info.outlined)
    {
        DrawScope outlined_scope(manager, id);
        manager->set_mvp_matrix(info.mvp);
        manager->set_color(info.outlined_color);
        manager->set_line_width(info.outlined_width);
        manager->set_depth_priority(info.depth_priority);
        manager->mesh(ViewportUiDrawManager::PrimitiveTypeLinesLoop, vertices);
    }
}

//////////////////////////////////////////////////////////////////////////
// draw_triangle
//////////////////////////////////////////////////////////////////////////

void draw_triangle(ViewportUiDrawManager* manager, const TriangleInfo& info, uint32_t id)
{
    std::vector<GfVec3f> vertices;
    vertices.push_back(GfVec3f(info.a[0], info.a[1], 0.0f));
    vertices.push_back(GfVec3f(info.b[0], info.b[1], 0.0f));
    vertices.push_back(GfVec3f(info.c[0], info.c[1], 0.0f));

    DrawScope quad_scope(manager, id);
    manager->set_mvp_matrix(info.mvp);
    manager->set_color(info.color);
    manager->mesh(ViewportUiDrawManager::PrimitiveTypeTriangleFan, vertices);
}

//////////////////////////////////////////////////////////////////////////
// utils
//////////////////////////////////////////////////////////////////////////

PXR_NS::GfVec2f screen_to_clip(const int x, const int y, const int width, const int height)
{
    return GfVec2f((2.0f * x) / width - 1.0f, 1.0f - (2.0f * y) / height);
}

PXR_NS::GfVec2f screen_to_clip(const PXR_NS::GfVec2f pos, const int width, const int height)
{
    return screen_to_clip(pos[0], pos[1], width, height);
}

PXR_NS::GfVec2f screen_to_world(const int x, const int y, const PXR_NS::GfFrustum& frustum, const int width, const int height)
{
    const auto frustum_nearfar = frustum.GetNearFar();
    const auto frustum_near = frustum_nearfar.GetMin();

    const auto corners = frustum.ComputeCornersAtDistance(frustum_near);

    const auto left_bottom = corners[0];
    const auto right_top = corners[3];

    const float x_left = left_bottom[0];
    const float x_right = right_top[0];
    const float y_bottom = left_bottom[1];
    const float y_top = right_top[1];

    return { x_left + (x_right - x_left) * static_cast<float>(x) / width, y_top - (y_top - y_bottom) * static_cast<float>(y) / height };
}

PXR_NS::GfVec2f screen_to_world(PXR_NS::GfVec2f screen, const PXR_NS::GfFrustum& frustum, const int width, const int height)
{
    return screen_to_world(screen[0], screen[1], frustum, width, height);
}

PXR_NS::GfVec2f world_to_screen(const int x, const int y, const PXR_NS::GfFrustum& frustum, const int width, const int height)
{
    const auto frustum_nearfar = frustum.GetNearFar();
    const auto frustum_near = frustum_nearfar.GetMin();

    const auto corners = frustum.ComputeCornersAtDistance(frustum_near);

    const auto left_bottom = corners[0];
    const auto right_top = corners[3];

    const float x_left = left_bottom[0];
    const float x_right = right_top[0];
    const float y_bottom = left_bottom[1];
    const float y_top = right_top[1];

    return { (static_cast<float>(x) - x_left) / (x_right - x_left) * width, -(static_cast<float>(y) - y_top) / (y_top - y_bottom) * height };
}

PXR_NS::GfVec2f world_to_screen(PXR_NS::GfVec2f world, const PXR_NS::GfFrustum& frustum, const int width, const int height)
{
    return world_to_screen(world[0], world[1], frustum, width, height);
}

PXR_NS::UsdTimeCode get_non_varying_time(const PXR_NS::UsdGeomPrimvar& primvar)
{
    if (!primvar)
        return PXR_NS::UsdTimeCode::Default();

    std::vector<double> time_samples;
    if (!primvar.GetTimeSamples(&time_samples))
    {
        return PXR_NS::UsdTimeCode::Default();
    }
    else if (time_samples.size() == 1)
    {
        return time_samples[0];
    }

    return PXR_NS::UsdTimeCode::Default();
}

OPENDCC_NAMESPACE_CLOSE
