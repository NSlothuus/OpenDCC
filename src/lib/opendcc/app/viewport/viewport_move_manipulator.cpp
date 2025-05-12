// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_move_manipulator.h"
#include "opendcc/app/viewport/viewport_move_snap_strategy.h"
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include <qevent.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include "opendcc/app/viewport/viewport_manipulator_utils.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    static const GfVec4f g_x_color(1, 0, 0, 1);
    static const GfVec4f g_y_color(0, 1, 0, 1);
    static const GfVec4f g_z_color(0, 0, 1, 1);
    static const GfVec4f g_select_color(1, 1, 0, 1);
    static const GfVec4f g_locate_color(1, 0.75, 0.5, 1);
    static const GfVec4f g_locate_color_transparent(1, 0.75, 0.5, 0.5);
    static const GfVec4f g_xy_color(0, 0, 1, 1);
    static const GfVec4f g_xy_color_transparent(0, 0, 1, 0.4);
    static const GfVec4f g_xz_color(0, 1, 0, 1);
    static const GfVec4f g_xz_color_transparent(0, 1, 0, 0.4);
    static const GfVec4f g_yz_color(1, 0, 0, 1);
    static const GfVec4f g_yz_color_transparent(1, 0, 0, 0.4);
    static const GfVec4f g_xyz_color(.392, 0.863, 1, 1);
    static const GfVec4f g_xyz_color_transparent(.392, 0.863, 1, 0.4);
    static const GfVec4f g_xyz_select_color_transparent(1, 1, 0, 0.5);
    static const GfVec4f g_lock_color(0.4, 0.4, 0.4, 1);
    static const GfVec4f g_lock_color_transparent(0.4, 0.4, 0.4, 0.4);
};

void ViewportMoveManipulator::on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                             ViewportUiDrawManager* draw_manager)
{
    using namespace manipulator_utils;
    if (!viewport_view || is_locked())
        return;

    m_move_mode = MoveMode::NONE;
    auto iter = m_handle_id_to_axis.find(draw_manager->get_current_selection());
    if (iter == m_handle_id_to_axis.end())
        return;
    m_move_mode = iter->second;

    static std::unordered_map<MoveMode, GfVec3d> directions = { { MoveMode::X, GfVec3d(1, 0, 0) },  { MoveMode::Y, GfVec3d(0, 1, 0) },
                                                                { MoveMode::Z, GfVec3d(0, 0, 1) },  { MoveMode::XZ, GfVec3d(0, 1, 0) },
                                                                { MoveMode::XY, GfVec3d(0, 0, 1) }, { MoveMode::YZ, GfVec3d(1, 0, 0) } };

    static std::unordered_map<MoveMode,
                              std::function<bool(const ViewportViewPtr&, const GfVec3d&, const GfVec3d&, const GfMatrix4d&, int, int, GfVec3d&)>>
        delta_computations = { { MoveMode::X, &compute_axis_intersection },   { MoveMode::Y, &compute_axis_intersection },
                               { MoveMode::Z, &compute_axis_intersection },   { MoveMode::XZ, &compute_plane_intersection },
                               { MoveMode::XY, &compute_plane_intersection }, { MoveMode::YZ, &compute_plane_intersection },
                               { MoveMode::XYZ, &compute_plane_intersection } };

    if (m_move_mode == MoveMode::XYZ)
    {
        m_drag_direction = compute_pick_ray(viewport_view, mouse_event.x(), mouse_event.y()).GetDirection();
    }
    else
    {
        auto orth = m_gizmo_matrix.GetOrthonormalized();
        m_drag_direction = (directions[m_move_mode] * orth.ExtractRotationMatrix()).GetNormalized();
    }

    m_drag_plane_translation = m_gizmo_matrix.ExtractTranslation();
    m_compute_intersection_point = delta_computations[m_move_mode];

    m_compute_intersection_point(viewport_view, m_drag_plane_translation, m_drag_direction, m_view_projection, mouse_event.x(), mouse_event.y(),
                                 m_start_drag_point);
}

void ViewportMoveManipulator::on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                            ViewportUiDrawManager* draw_manager)
{
    if (m_move_mode == MoveMode::NONE || is_locked())
        return;

    GfVec3d intersection_point;
    if (m_compute_intersection_point(viewport_view, m_drag_plane_translation, m_drag_direction, m_view_projection, mouse_event.x(), mouse_event.y(),
                                     intersection_point))
    {
        if (m_snap_strategy)
        {
            const auto snap_world_pos = m_snap_strategy->get_snap_point(m_drag_plane_translation, m_start_drag_point, intersection_point);
            if (m_move_mode == MoveMode::X || m_move_mode == MoveMode::Y || m_move_mode == MoveMode::Z)
            {
                // Project onto move direction
                m_delta = (snap_world_pos - m_drag_plane_translation).GetProjection(m_drag_direction);
            }
            else if (m_move_mode == MoveMode::XZ || m_move_mode == MoveMode::XY || m_move_mode == MoveMode::YZ)
            {
                // Project onto move plane
                m_delta = snap_world_pos + (m_drag_plane_translation - snap_world_pos).GetProjection(m_drag_direction) - m_drag_plane_translation;
            }
            else
            {
                m_delta = snap_world_pos - m_drag_plane_translation;
            }
        }
        else
        {
            m_delta = intersection_point - m_start_drag_point;
        }

        m_gizmo_matrix.SetTranslateOnly(m_drag_plane_translation + get_delta());
    }
}

void ViewportMoveManipulator::on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                               ViewportUiDrawManager* draw_manager)
{
    m_move_mode = MoveMode::NONE;
    m_delta.Set(0, 0, 0);
}

void ViewportMoveManipulator::draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view || m_gizmo_matrix == GfMatrix4d(0.0))
        return;

    static const GfVec3f orig(0, 0, 0);
    static const GfVec3f axis_x(1, 0, 0);
    static const GfVec3f axis_y(0, 1, 0);
    static const GfVec3f axis_z(0, 0, 1);
    GfCamera camera = viewport_view->get_camera();
    GfFrustum frustum = camera.GetFrustum();

    const auto gizmo_center = m_gizmo_matrix.ExtractTranslation();
    const double screen_factor = manipulator_utils::compute_screen_factor(viewport_view, GfVec3d(gizmo_center[0], gizmo_center[1], gizmo_center[2]));

    const auto viewport_dim = viewport_view->get_viewport_dimensions();
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit,
                            viewport_dim.height != 0 ? (double)viewport_dim.width / viewport_dim.height : 1.0);

    if (GfIsClose(gizmo_center, frustum.GetPosition(), 0.00001))
        return;

    GfMatrix4d proj_matrix = frustum.ComputeProjectionMatrix();
    GfMatrix4d model_matrix = GfMatrix4d().SetScale(screen_factor) * m_gizmo_matrix;
    m_view_projection = frustum.ComputeViewMatrix() * proj_matrix;
    GfMatrix4d vp_matrix = model_matrix * m_view_projection;
    GfMatrix4f vp_matrixf = GfMatrix4f(vp_matrix);

    if (m_handle_id_to_axis.empty())
        init_handle_ids(draw_manager);

    auto colors = assign_colors(draw_manager->get_current_selection());

    const auto view_dir = m_gizmo_matrix.GetInverse().Transform(frustum.GetPosition()).GetNormalized();
    if (GfAbs(GfDot(view_dir, axis_x)) < 0.99)
        draw_utils::draw_axis(draw_manager, vp_matrixf, GfMatrix4f(model_matrix), *(colors[MoveMode::X].color), orig, axis_x, axis_y, axis_z, 0.05f,
                              0.83f, m_axis_to_handle_id[MoveMode::X]);
    if (GfAbs(GfDot(view_dir, axis_y)) < 0.99)
        draw_utils::draw_axis(draw_manager, vp_matrixf, GfMatrix4f(model_matrix), *(colors[MoveMode::Y].color), orig, axis_y, axis_z, axis_x, 0.05f,
                              0.83f, m_axis_to_handle_id[MoveMode::Y]);
    if (GfAbs(GfDot(view_dir, axis_z)) < 0.99)
        draw_utils::draw_axis(draw_manager, vp_matrixf, GfMatrix4f(model_matrix), *(colors[MoveMode::Z].color), orig, axis_z, axis_x, axis_y, 0.05f,
                              0.83f, m_axis_to_handle_id[MoveMode::Z]);

    const double rect_size = 0.1 * screen_factor;
    GfMatrix4d rect_matrix = GfMatrix4d().SetTranslate(gizmo_center) * frustum.ComputeViewMatrix();
    rect_matrix.SetTranslate(rect_matrix.ExtractTranslation());

    rect_matrix = rect_matrix * frustum.ComputeProjectionMatrix();

    std::vector<GfVec3f> xyz_quad = { GfVec3f(-rect_size, -rect_size, 0), GfVec3f(rect_size, -rect_size, 0), GfVec3f(rect_size, rect_size, 0),
                                      GfVec3f(-rect_size, rect_size, 0) };

    if (m_snap_strategy == nullptr)
    {
        draw_utils::draw_outlined_quad(draw_manager, GfMatrix4f(rect_matrix), *(colors[MoveMode::XYZ].transparent), *(colors[MoveMode::XYZ].color),
                                       xyz_quad, 1.0f, 1, m_axis_to_handle_id[MoveMode::XYZ]);
    }
    else
    {
        GfMatrix4d model_matrix = GfMatrix4d().SetScale(0.1 * screen_factor);
        GfMatrix4f mvp = GfMatrix4f(model_matrix.SetTranslateOnly(m_gizmo_matrix.ExtractTranslation()) * m_view_projection);
        const auto up = GfVec3f(frustum.ComputeUpVector()).GetNormalized();
        const auto right = GfVec3f(up ^ frustum.ComputeViewDirection()).GetNormalized();
        draw_utils::draw_outlined_circle(draw_manager, mvp, *(colors[MoveMode::XYZ].transparent), *(colors[MoveMode::XYZ].color), GfVec3f(0, 0, 0),
                                         right, up, 1.0f, 1, m_axis_to_handle_id[MoveMode::XYZ]);
    }
    static const std::vector<GfVec3f> xy_quad = { GfVec3f(0.4f, 0.4f, 0), GfVec3f(0.6f, 0.4f, 0), GfVec3f(0.6f, 0.6f, 0), GfVec3f(0.4f, 0.6f, 0) };

    if (GfAbs(GfDot(view_dir, axis_z)) > 0.2)
    {
        draw_utils::draw_outlined_quad(draw_manager, vp_matrixf, *(colors[MoveMode::XY].transparent), *(colors[MoveMode::XY].color), xy_quad, 1.0f, 1,
                                       m_axis_to_handle_id[MoveMode::XY]);
    }

    static const std::vector<GfVec3f> xz_quad = { GfVec3f(0.4f, 0, 0.4f), GfVec3f(0.6f, 0, 0.4f), GfVec3f(0.6f, 0, 0.6f), GfVec3f(0.4f, 0, 0.6f) };

    if (GfAbs(GfDot(view_dir, axis_y)) > 0.2)
    {
        draw_utils::draw_outlined_quad(draw_manager, vp_matrixf, *(colors[MoveMode::XZ].transparent), *(colors[MoveMode::XZ].color), xz_quad, 1.0f, 1,
                                       m_axis_to_handle_id[MoveMode::XZ]);
    }

    static const std::vector<GfVec3f> yz_quad = { GfVec3f(0, 0.4f, 0.4f), GfVec3f(0, 0.6f, 0.4f), GfVec3f(0, 0.6f, 0.6f), GfVec3f(0, 0.4f, 0.6f) };

    if (GfAbs(GfDot(view_dir, axis_x)) > 0.2)
    {
        draw_utils::draw_outlined_quad(draw_manager, vp_matrixf, *(colors[MoveMode::YZ].transparent), *(colors[MoveMode::YZ].color), yz_quad, 1.0f, 1,
                                       m_axis_to_handle_id[MoveMode::YZ]);
    }
}

const GfMatrix4d& ViewportMoveManipulator::get_gizmo_matrix() const
{
    return m_gizmo_matrix;
}

GfVec3d ViewportMoveManipulator::get_delta() const
{
    return m_delta;
}

void ViewportMoveManipulator::set_gizmo_matrix(const GfMatrix4d& gizmo_matrix)
{
    m_gizmo_matrix = gizmo_matrix;
    m_delta = GfVec3f(0);
}

bool ViewportMoveManipulator::is_locked() const
{
    return m_is_locked;
}

void ViewportMoveManipulator::set_locked(bool locked)
{
    m_is_locked = locked;
}

void ViewportMoveManipulator::set_snap_strategy(std::shared_ptr<ViewportSnapStrategy> snap_strategy)
{
    m_snap_strategy = snap_strategy;
}

bool ViewportMoveManipulator::is_valid() const
{
    return GfMatrix4d().SetZero() != m_gizmo_matrix;
}

ViewportMoveManipulator::GizmoColors ViewportMoveManipulator::assign_colors(uint32_t selected_handle)
{
    static GizmoColors locked_colors;
    static std::unordered_map<MoveMode, GizmoColors> colors_per_mode;
    static std::once_flag once;
    std::call_once(once, [] {
        for (int i = 0; i < static_cast<int>(MoveMode::COUNT); i++)
        {
            auto mode = static_cast<MoveMode>(i);
            locked_colors[mode] = { &g_lock_color, &g_lock_color_transparent };
            GizmoColors colors;
            switch (mode)
            {
            case MoveMode::NONE:
            {
                colors[MoveMode::X].color = &g_x_color;
                colors[MoveMode::Y].color = &g_y_color;
                colors[MoveMode::Z].color = &g_z_color;
                colors[MoveMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[MoveMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[MoveMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[MoveMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            }
            case MoveMode::X:
                colors[MoveMode::X].color = &g_select_color;
                colors[MoveMode::Y].color = &g_y_color;
                colors[MoveMode::Z].color = &g_z_color;
                colors[MoveMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[MoveMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[MoveMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[MoveMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            case MoveMode::Y:
                colors[MoveMode::X].color = &g_x_color;
                colors[MoveMode::Y].color = &g_select_color;
                colors[MoveMode::Z].color = &g_z_color;
                colors[MoveMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[MoveMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[MoveMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[MoveMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            case MoveMode::Z:
                colors[MoveMode::X].color = &g_x_color;
                colors[MoveMode::Y].color = &g_y_color;
                colors[MoveMode::Z].color = &g_select_color;
                colors[MoveMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[MoveMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[MoveMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[MoveMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            case MoveMode::XY:
                colors[MoveMode::X].color = &g_select_color;
                colors[MoveMode::Y].color = &g_select_color;
                colors[MoveMode::Z].color = &g_z_color;
                colors[MoveMode::XY] = { &g_select_color, &g_select_color };
                colors[MoveMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[MoveMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[MoveMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            case MoveMode::XZ:
                colors[MoveMode::X].color = &g_select_color;
                colors[MoveMode::Y].color = &g_y_color;
                colors[MoveMode::Z].color = &g_select_color;
                colors[MoveMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[MoveMode::XZ] = { &g_select_color, &g_select_color };
                colors[MoveMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[MoveMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            case MoveMode::YZ:
                colors[MoveMode::X].color = &g_x_color;
                colors[MoveMode::Y].color = &g_select_color;
                colors[MoveMode::Z].color = &g_select_color;
                colors[MoveMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[MoveMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[MoveMode::YZ] = { &g_select_color, &g_select_color };
                colors[MoveMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            case MoveMode::XYZ:
                colors[MoveMode::X].color = &g_select_color;
                colors[MoveMode::Y].color = &g_select_color;
                colors[MoveMode::Z].color = &g_select_color;
                colors[MoveMode::XY] = { &g_xy_color, &g_xy_color_transparent };
                colors[MoveMode::XZ] = { &g_xz_color, &g_xz_color_transparent };
                colors[MoveMode::YZ] = { &g_yz_color, &g_yz_color_transparent };
                colors[MoveMode::XYZ] = { &g_select_color, &g_xyz_select_color_transparent };
                break;
            }
            colors_per_mode[mode] = colors;
        }
    });

    if (is_locked())
        return locked_colors;

    auto result = colors_per_mode[m_move_mode];
    if (m_move_mode == MoveMode::NONE)
    {
        auto iter = m_handle_id_to_axis.find(selected_handle);
        if (iter != m_handle_id_to_axis.end())
            result[iter->second] = { &g_locate_color, &g_locate_color_transparent };
    }

    return result;
}

void ViewportMoveManipulator::init_handle_ids(ViewportUiDrawManager* draw_manager)
{
    if (!draw_manager)
        return;

    for (int mode = 1; mode < static_cast<int>(MoveMode::COUNT); mode++)
    {
        auto insert_result = m_axis_to_handle_id.emplace(static_cast<MoveMode>(mode), draw_manager->create_selection_id());
        m_handle_id_to_axis[insert_result.first->second] = static_cast<MoveMode>(mode);
    }
}
OPENDCC_NAMESPACE_CLOSE
