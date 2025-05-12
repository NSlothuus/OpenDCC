// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_scale_manipulator.h"
#include "opendcc/app/viewport/viewport_manipulator_utils.h"
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include <qevent.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE
using namespace manipulator_utils;

namespace
{
    static const GfVec4f g_x_color(1, 0, 0, 1);
    static const GfVec4f g_y_color(0, 1, 0, 1);
    static const GfVec4f g_z_color(0, 0, 1, 1);
    static const GfVec4f g_select_color(1, 1, 0, 1);
    static const GfVec4f g_select_color_transparent(1, 1, 0, 0.5);
    static const GfVec4f g_locate_color(1, 0.75, 0.5, 1);
    static const GfVec4f g_locate_color_transparent(1, 0.75, 0.5, 0.5);
    static const GfVec4f g_xy_color(0, 0, 1, 1);
    static const GfVec4f g_xy_color_transparent(0, 0, 1, 0.4);
    static const GfVec4f g_xz_color(0, 1, 0, 1);
    static const GfVec4f g_xz_color_transparent(0, 1, 0, 0.4);
    static const GfVec4f g_yz_color(1, 0, 0, 1);
    static const GfVec4f g_yz_color_transparent(1, 0, 0, 0.4);
    static const GfVec4f g_xyz_color(.392, 0.863, 1, 1);
    static const GfVec4f g_xyz_color_transparent(.392, 0.863, 1, 0);
    static const GfVec4f g_xyz_select_color_transparent(1, 1, 0, 0);
    static const GfVec4f g_xyz_locate_color_transparent(1, 0.75, 0.5, 0);
    static const GfVec4f g_lock_color(0.4, 0.4, 0.4, 1);
    static const GfVec4f g_lock_color_transparent(0.4, 0.4, 0.4, 0.4);
};

void ViewportScaleManipulator::on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                              ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view || is_locked())
        return;

    m_scale_mode = ScaleMode::NONE;
    auto iter = m_handle_id_to_axis.find(draw_manager->get_current_selection());
    if (iter == m_handle_id_to_axis.end())
        return;
    m_scale_mode = iter->second;

    static std::unordered_map<ScaleMode, GfVec3d> directions = {
        { ScaleMode::X, GfVec3d(1, 0, 0) },   { ScaleMode::Y, GfVec3d(0, 1, 0) },  { ScaleMode::Z, GfVec3d(0, 0, 1) },
        { ScaleMode::XY, GfVec3d(0, 0, 1) },  { ScaleMode::XZ, GfVec3d(0, 1, 0) }, { ScaleMode::YZ, GfVec3d(1, 0, 0) },
        { ScaleMode::XYZ, GfVec3d(0, 0, 0) },
    };

    static std::unordered_map<ScaleMode,
                              std::function<bool(const ViewportViewPtr&, const GfVec3d&, const GfVec3d&, const GfMatrix4d&, int, int, GfVec3d&)>>
        delta_computations = { { ScaleMode::X, &compute_axis_intersection },   { ScaleMode::Y, &compute_axis_intersection },
                               { ScaleMode::Z, &compute_axis_intersection },   { ScaleMode::XY, &compute_plane_intersection },
                               { ScaleMode::XZ, &compute_plane_intersection }, { ScaleMode::YZ, &compute_plane_intersection },
                               { ScaleMode::XYZ, &compute_screen_space_pos } };

    m_drag_direction = (directions[m_scale_mode] * m_gizmo_data.gizmo_matrix.ExtractRotationMatrix()).GetNormalized();
    m_compute_intersection_point = delta_computations[m_scale_mode];

    if (m_scale_mode == ScaleMode::XYZ)
    {
        m_drag_direction = compute_pick_ray(viewport_view, mouse_event.x(), mouse_event.y()).GetDirection();
    }

    if (m_compute_intersection_point(viewport_view, m_gizmo_data.gizmo_matrix.ExtractTranslation(), m_drag_direction, m_view_projection,
                                     mouse_event.x(), mouse_event.y(), m_start_drag_point))
    {
        if (m_scale_mode != ScaleMode::XYZ)
        {
            const auto gizmo_center = m_gizmo_data.gizmo_matrix.ExtractTranslation();
            const auto screen_factor = compute_screen_factor(viewport_view, gizmo_center);
            m_inv_gizmo_matrix = (GfMatrix4d().SetScale(screen_factor) * m_gizmo_data.gizmo_matrix).GetInverse();
            m_start_drag_point = m_inv_gizmo_matrix.Transform(m_start_drag_point);
        }
    }
}

void ViewportScaleManipulator::on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                             ViewportUiDrawManager* draw_manager)
{
    if (m_scale_mode == ScaleMode::NONE || is_locked())
        return;

    GfVec3d intersection_point;
    if (m_compute_intersection_point(viewport_view, m_gizmo_data.gizmo_matrix.ExtractTranslation(), m_drag_direction, m_view_projection,
                                     mouse_event.x(), mouse_event.y(), intersection_point))
    {
        GfVec3d delta;
        if (m_scale_mode == ScaleMode::X || m_scale_mode == ScaleMode::Y || m_scale_mode == ScaleMode::Z)
        {
            intersection_point = m_inv_gizmo_matrix.Transform(intersection_point);
            delta = intersection_point - m_start_drag_point;
        }
        else if (m_scale_mode == ScaleMode::XYZ)
        {
            delta = GfVec3d(5 * (intersection_point[0] - m_start_drag_point[0]));
        }
        else
        {
            intersection_point = m_inv_gizmo_matrix.Transform(intersection_point);
            delta = intersection_point - m_start_drag_point;
            delta = GfVec3d(delta[0] + delta[1] + delta[2]);
            switch (m_scale_mode)
            {
            case ScaleMode::XY:
                delta[2] = 0;
                break;
            case ScaleMode::XZ:
                delta[1] = 0;
                break;
            case ScaleMode::YZ:
                delta[0] = 0;
                break;
            }
        }

        m_delta = GfVec3f(delta);
    }
}

void ViewportScaleManipulator::on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                                ViewportUiDrawManager* draw_manager)
{
    m_scale_mode = ScaleMode::NONE;
    m_delta = GfVec3f(0);
}

void ViewportScaleManipulator::draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view || m_gizmo_data.gizmo_matrix == GfMatrix4d(0.0))
        return;

    const auto delta = get_delta();
    const GfVec3f axis_x = GfCompMult(GfVec3f(1, 0, 0), delta);
    const GfVec3f axis_y = GfCompMult(GfVec3f(0, 1, 0), delta);
    const GfVec3f axis_z = GfCompMult(GfVec3f(0, 0, 1), delta);
    GfCamera camera = viewport_view->get_camera();
    GfFrustum frustum = camera.GetFrustum();

    const auto gizmo_center = m_gizmo_data.gizmo_matrix.ExtractTranslation();
    const double screen_factor = compute_screen_factor(viewport_view, m_gizmo_data.gizmo_matrix.ExtractTranslation());

    const auto viewport_dim = viewport_view->get_viewport_dimensions();
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit,
                            viewport_dim.height != 0 ? (double)viewport_dim.width / viewport_dim.height : 1.0);

    if (GfIsClose(gizmo_center, frustum.GetPosition(), 0.00001))
        return;

    GfMatrix4d proj_matrix = frustum.ComputeProjectionMatrix();
    m_view_projection = frustum.ComputeViewMatrix() * proj_matrix;
    GfMatrix4d model_matrix = GfMatrix4d().SetScale(screen_factor) * m_gizmo_data.gizmo_matrix;
    GfMatrix4d vp_matrix = model_matrix * m_view_projection;
    GfMatrix4f vp_matrixf = GfMatrix4f(vp_matrix);

    if (m_handle_id_to_axis.empty())
        init_handle_ids(draw_manager);

    auto colors = assign_colors(draw_manager->get_current_selection());
    const auto view_dir = m_gizmo_data.gizmo_matrix.GetInverse().Transform(frustum.GetPosition()).GetNormalized();

    if (GfAbs(GfDot(view_dir, axis_x.GetNormalized())) < 0.99)
        draw_axis(draw_manager, vp_matrixf, GfMatrix4f(model_matrix), *colors[ScaleMode::X].color, axis_x, m_axis_to_handle_id[ScaleMode::X]);
    if (GfAbs(GfDot(view_dir, axis_y.GetNormalized())) < 0.99)
        draw_axis(draw_manager, vp_matrixf, GfMatrix4f(model_matrix), *colors[ScaleMode::Y].color, axis_y, m_axis_to_handle_id[ScaleMode::Y]);
    if (GfAbs(GfDot(view_dir, axis_z.GetNormalized())) < 0.99)
        draw_axis(draw_manager, vp_matrixf, GfMatrix4f(model_matrix), *colors[ScaleMode::Z].color, axis_z, m_axis_to_handle_id[ScaleMode::Z]);

    std::vector<GfVec3f> xy_quad = { GfVec3f(0.4f, 0.4f, 0), GfVec3f(0.6f, 0.4f, 0), GfVec3f(0.6f, 0.6f, 0), GfVec3f(0.4f, 0.6f, 0) };
    auto xy_center = std::accumulate(xy_quad.begin(), xy_quad.end(), GfVec3f(0.0f)) / xy_quad.size();
    xy_center = GfCompMult(xy_center, delta) - xy_center;
    for (auto& xy : xy_quad)
    {
        xy = xy_center + xy;
    }
    if (GfAbs(GfDot(view_dir, axis_z.GetNormalized())) > 0.2 && (m_scale_mode == ScaleMode::XY || m_scale_mode == ScaleMode::NONE))
        draw_utils::draw_outlined_quad(draw_manager, vp_matrixf, *colors[ScaleMode::XY].transparent, *colors[ScaleMode::XY].color, xy_quad, 1, 1,
                                       m_axis_to_handle_id[ScaleMode::XY]);

    std::vector<GfVec3f> xz_quad = { GfVec3f(0.4f, 0, 0.4f), GfVec3f(0.6f, 0, 0.4f), GfVec3f(0.6f, 0, 0.6f), GfVec3f(0.4f, 0, 0.6f) };
    auto xz_center = std::accumulate(xz_quad.begin(), xz_quad.end(), GfVec3f(0.0f)) / xz_quad.size();
    xz_center = GfCompMult(xz_center, delta) - xz_center;
    for (auto& xz : xz_quad)
    {
        xz = xz_center + xz;
    }
    if (GfAbs(GfDot(view_dir, axis_y.GetNormalized())) > 0.2 && (m_scale_mode == ScaleMode::XZ || m_scale_mode == ScaleMode::NONE))
        draw_utils::draw_outlined_quad(draw_manager, vp_matrixf, *colors[ScaleMode::XZ].transparent, *colors[ScaleMode::XZ].color, xz_quad, 1, 1,
                                       m_axis_to_handle_id[ScaleMode::XZ]);

    std::vector<GfVec3f> yz_quad = { GfVec3f(0, 0.4f, 0.4f), GfVec3f(0, 0.6f, 0.4f), GfVec3f(0, 0.6f, 0.6f), GfVec3f(0, 0.4f, 0.6f) };
    auto yz_center = std::accumulate(yz_quad.begin(), yz_quad.end(), GfVec3f(0.0f)) / yz_quad.size();
    yz_center = GfCompMult(yz_center, delta) - yz_center;
    for (auto& yz : yz_quad)
    {
        yz = yz_center + yz;
    }
    if (GfAbs(GfDot(view_dir, axis_x.GetNormalized())) > 0.2 && (m_scale_mode == ScaleMode::YZ || m_scale_mode == ScaleMode::NONE))
        draw_utils::draw_outlined_quad(draw_manager, vp_matrixf, *colors[ScaleMode::YZ].transparent, *colors[ScaleMode::YZ].color, yz_quad, 1, 1,
                                       m_axis_to_handle_id[ScaleMode::YZ]);

    draw_utils::draw_cube(draw_manager, vp_matrixf, GfMatrix4f(model_matrix), *colors[ScaleMode::XYZ].color, 0.05, 1,
                          m_axis_to_handle_id[ScaleMode::XYZ]);
}

GfVec3f ViewportScaleManipulator::get_delta() const
{
    if (m_step_mode == StepMode::OFF)
        return m_delta + GfVec3f(1);

    GfVec3f result = m_delta;
    for (int i = 0; i < result.dimension; i++)
    {
        if (GfIsClose(result[i], 0, 0.000001))
        {
            result[i] = 1.0;
        }
        else
        {
            const auto div = result[i] / m_step;
            const auto integer = static_cast<int64_t>(div);
            auto delta = m_step * integer + m_step * std::ceil(div - integer);
            result[i] = m_step_mode == StepMode::ABSOLUTE_MODE ? (delta + m_gizmo_data.scale[i]) / m_gizmo_data.scale[i]
                        : GfIsClose(delta, 0, 0.000001)        ? 1.0
                                                               : delta;
        }
    }
    return result;
}

bool ViewportScaleManipulator::is_locked() const
{
    return m_is_locked;
}

void ViewportScaleManipulator::set_locked(bool locked)
{
    m_is_locked = locked;
}

void ViewportScaleManipulator::set_gizmo_data(const GizmoData& gizmo_data)
{
    m_gizmo_data = gizmo_data;
    m_delta = GfVec3f(0);
}

ViewportScaleManipulator::ScaleMode ViewportScaleManipulator::get_scale_mode() const
{
    return m_scale_mode;
}

ViewportScaleManipulator::StepMode ViewportScaleManipulator::get_step_mode() const
{
    return m_step_mode;
}

void ViewportScaleManipulator::set_step_mode(StepMode mode)
{
    m_step_mode = mode;
}

double ViewportScaleManipulator::get_step() const
{
    return m_step;
}

void ViewportScaleManipulator::set_step(double step)
{
    if (GfIsClose(step, 0, 0.000001))
        step = 1.0;
    m_step = std::abs(step);
}

bool ViewportScaleManipulator::is_valid() const
{
    return GfMatrix4d().SetZero() != m_gizmo_data.gizmo_matrix;
}

ViewportScaleManipulator::GizmoColors ViewportScaleManipulator::assign_colors(uint32_t selected_handle)
{
    static GizmoColors locked_colors;
    static std::unordered_map<ScaleMode, GizmoColors> colors_per_mode;
    static std::once_flag once;
    std::call_once(once, [] {
        for (int i = 0; i < static_cast<int>(ScaleMode::COUNT); i++)
        {
            auto mode = static_cast<ScaleMode>(i);
            locked_colors[mode] = { &g_lock_color, &g_lock_color_transparent };
            GizmoColors colors;
            switch (mode)
            {
            case ScaleMode::NONE:
                colors[ScaleMode::X].color = &g_x_color;
                colors[ScaleMode::Y].color = &g_y_color;
                colors[ScaleMode::Z].color = &g_z_color;
                colors[ScaleMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[ScaleMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[ScaleMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[ScaleMode::XYZ].color = &g_xyz_color;
                break;
            case ScaleMode::X:
                colors[ScaleMode::X].color = &g_select_color;
                colors[ScaleMode::Y].color = &g_y_color;
                colors[ScaleMode::Z].color = &g_z_color;
                colors[ScaleMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[ScaleMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[ScaleMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[ScaleMode::XYZ].color = &g_xyz_color;
                break;
            case ScaleMode::Y:
                colors[ScaleMode::X].color = &g_x_color;
                colors[ScaleMode::Y].color = &g_select_color;
                colors[ScaleMode::Z].color = &g_z_color;
                colors[ScaleMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[ScaleMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[ScaleMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[ScaleMode::XYZ].color = &g_xyz_color;
                break;
            case ScaleMode::Z:
                colors[ScaleMode::X].color = &g_x_color;
                colors[ScaleMode::Y].color = &g_y_color;
                colors[ScaleMode::Z].color = &g_select_color;
                colors[ScaleMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[ScaleMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[ScaleMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[ScaleMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            case ScaleMode::XY:
                colors[ScaleMode::X].color = &g_select_color;
                colors[ScaleMode::Y].color = &g_select_color;
                colors[ScaleMode::Z].color = &g_z_color;
                colors[ScaleMode::XY] = { &g_select_color, &g_select_color };
                colors[ScaleMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[ScaleMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[ScaleMode::XYZ].color = &g_xyz_color;
                break;
            case ScaleMode::XZ:
                colors[ScaleMode::X].color = &g_select_color;
                colors[ScaleMode::Y].color = &g_y_color;
                colors[ScaleMode::Z].color = &g_select_color;
                colors[ScaleMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[ScaleMode::XZ] = { &g_select_color, &g_select_color };
                colors[ScaleMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[ScaleMode::XYZ].color = &g_xyz_color;
                break;
            case ScaleMode::YZ:
                colors[ScaleMode::X].color = &g_x_color;
                colors[ScaleMode::Y].color = &g_select_color;
                colors[ScaleMode::Z].color = &g_select_color;
                colors[ScaleMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[ScaleMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[ScaleMode::YZ] = { &g_select_color, &g_select_color };
                colors[ScaleMode::XYZ].color = &g_xyz_color;
                break;
            case ScaleMode::XYZ:
                colors[ScaleMode::X].color = &g_select_color;
                colors[ScaleMode::Y].color = &g_select_color;
                colors[ScaleMode::Z].color = &g_select_color;
                colors[ScaleMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[ScaleMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[ScaleMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[ScaleMode::XYZ].color = &g_select_color;
                break;
            }
            colors_per_mode[mode] = colors;
        }
    });

    if (m_is_locked)
        return locked_colors;

    auto result = colors_per_mode[m_scale_mode];
    if (m_scale_mode == ScaleMode::NONE)
    {
        auto iter = m_handle_id_to_axis.find(selected_handle);
        if (iter != m_handle_id_to_axis.end())
            result[iter->second] = { &g_locate_color, &g_locate_color_transparent };
    }

    return result;
}

void ViewportScaleManipulator::init_handle_ids(ViewportUiDrawManager* draw_manager)
{
    if (!draw_manager)
        return;

    for (int mode = 1; mode < static_cast<int>(ScaleMode::COUNT); mode++)
    {
        auto insert_result = m_axis_to_handle_id.emplace(static_cast<ScaleMode>(mode), draw_manager->create_selection_id());
        m_handle_id_to_axis[insert_result.first->second] = static_cast<ScaleMode>(mode);
    }
}

void ViewportScaleManipulator::draw_axis(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& mvp, const PXR_NS::GfMatrix4f& model,
                                         const PXR_NS::GfVec4f& color, const PXR_NS::GfVec3f& axis, uint32_t axis_id)
{
    static const double cube_size = 0.05;
    static const GfVec3f orig(0);
    const auto axis_translate = GfMatrix4f().SetTranslate(axis);

    draw_manager->begin_drawable(axis_id);
    draw_manager->set_color(color);
    draw_manager->set_mvp_matrix(mvp);
    draw_manager->line(orig, orig + axis);
    draw_manager->set_line_width(2);
    draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLines);
    draw_manager->end_drawable();
    draw_utils::draw_cube(draw_manager, axis_translate * mvp, GfMatrix4f(axis_translate * model), color, cube_size, 0, axis_id);
}

OPENDCC_NAMESPACE_CLOSE
