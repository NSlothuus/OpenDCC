// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_rotate_manipulator.h"
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include "opendcc/app/viewport/viewport_manipulator_utils.h"
#include <qevent.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/vec3d.h>
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
    static const GfVec4f g_locate_color(1, 0.75, 0.5, 1);
    static const GfVec4f g_view_color(100 / 255.0, 220 / 255.0, 1, 1);
    static const GfVec4f g_xyz_color(64 / 255.0, 64 / 255.0, 64 / 255.0, 1);
    static const GfVec4f g_xyz_color_transparent(0, 0, 0, 0);
    static const GfVec4f g_xyz_locate_color_transparent(0.25);
    static const GfVec4f g_lock_color(0.4, 0.4, 0.4, 1);
    static const GfVec4f g_lock_color_transparent(0);
    static const GfVec3f g_pie_color(203 / 255.0);

    static int find_axis_index(int i, const std::array<int, 3>& axis_indices)
    {
        return std::distance(axis_indices.begin(), std::find(axis_indices.begin(), axis_indices.end(), i));
    }
    static GfMatrix3f get_axis_transform(int axis_ind, const GfMatrix3f& parent_transform, const std::array<GfMatrix3f, 3>& precomputed_rotations,
                                         const std::array<int, 3>& axis_indices)
    {
        GfMatrix3f result(1);
        for (int i = find_axis_index(axis_ind, axis_indices) + 1; i < 3; i++)
        {
            result *= precomputed_rotations[axis_indices[i]];
        }
        return result * parent_transform;
    };
};

void ViewportRotateManipulator::on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                               ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view || is_locked())
        return;

    m_rotate_mode = RotateMode::NONE;
    auto iter = m_handle_id_to_axis.find(draw_manager->get_current_selection());
    if (iter == m_handle_id_to_axis.end())
        return;

    m_rotate_mode = iter->second;
    if (m_orientation == Orientation::GIMBAL && m_rotate_mode == RotateMode::XYZ)
        return;

    const auto gizmo_center = m_gizmo_data.gizmo_matrix.ExtractTranslation();
    const auto intersection_point = compute_sphere_intersection(viewport_view, compute_screen_factor(viewport_view, gizmo_center), gizmo_center,
                                                                mouse_event.x(), mouse_event.y());

    m_start_vector = intersection_point - m_gizmo_data.gizmo_matrix.ExtractTranslation();
    m_start_gizmo_angles = m_gizmo_data.gizmo_angles;

    const auto indices = get_basis_indices_from_rot_order(m_gizmo_data.rotation_order);
    const auto x_rot = GfMatrix3f(GfRotation(GfVec3f::XAxis(), m_gizmo_data.gizmo_angles[0]));
    const auto y_rot = GfMatrix3f(GfRotation(GfVec3f::YAxis(), m_gizmo_data.gizmo_angles[1]));
    const auto z_rot = GfMatrix3f(GfRotation(GfVec3f::ZAxis(), m_gizmo_data.gizmo_angles[2]));
    const auto parent_rotation = GfMatrix3f(m_gizmo_data.parent_gizmo_matrix.GetOrthonormalized().ExtractRotationMatrix());
    switch (m_rotate_mode)
    {
    case RotateMode::X:
        m_axis = m_orientation == Orientation::GIMBAL ? GfVec3f::XAxis() * get_axis_transform(0, parent_rotation, { x_rot, y_rot, z_rot }, indices)
                                                      : GfVec3f::XAxis();
        break;
    case RotateMode::Y:
        m_axis = m_orientation == Orientation::GIMBAL ? GfVec3f::YAxis() * get_axis_transform(1, parent_rotation, { x_rot, y_rot, z_rot }, indices)
                                                      : GfVec3f::YAxis();
        break;
    case RotateMode::Z:
        m_axis = m_orientation == Orientation::GIMBAL ? GfVec3f::ZAxis() * get_axis_transform(2, parent_rotation, { x_rot, y_rot, z_rot }, indices)
                                                      : GfVec3f::ZAxis();
        break;
    case RotateMode::VIEW:
        m_axis = -GfVec3f(m_inv_start_matrix.TransformDir(compute_view_frustum(viewport_view).ComputeViewDirection()));
        break;
    case RotateMode::XYZ:
        m_axis.Set(0, 0, 0);
        break;
    }
    m_axis.Normalize();
}

void ViewportRotateManipulator::on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                              ViewportUiDrawManager* draw_manager)
{
    if (m_rotate_mode == RotateMode::NONE || is_locked() || (m_rotate_mode == RotateMode::XYZ && m_orientation == Orientation::GIMBAL))
        return;

    const auto gizmo_center = m_gizmo_data.gizmo_matrix.ExtractTranslation();
    const auto intersection_point = compute_sphere_intersection(viewport_view, compute_screen_factor(viewport_view, gizmo_center), gizmo_center,
                                                                mouse_event.x(), mouse_event.y());

    const auto end_vector = intersection_point - m_gizmo_data.gizmo_matrix.ExtractTranslation();
    if (m_orientation == Orientation::GIMBAL)
    {
        m_delta = GfRotation::RotateOntoProjected(m_start_vector, end_vector, m_axis);
        auto local_axis = GfVec3f(0);
        const auto axis_ind = static_cast<int>(m_rotate_mode) - 1;
        local_axis[axis_ind] = 1;
        if (!m_is_gizmo_locked)
            m_gizmo_data.gizmo_angles = m_start_gizmo_angles + local_axis * get_delta().GetAngle();
    }
    else
    {
        const auto transformed_start_vector = m_inv_start_matrix.TransformDir(m_start_vector.GetNormalized());
        const auto transformed_end_vector = m_inv_start_matrix.TransformDir(end_vector.GetNormalized());
        if (m_rotate_mode == RotateMode::XYZ)
        {
            m_delta = GfRotation(transformed_start_vector, transformed_end_vector);
        }
        else
        {
            m_delta = GfRotation::RotateOntoProjected(transformed_start_vector, transformed_end_vector, m_axis);
        }

        if (m_orientation == Orientation::OBJECT && !m_is_gizmo_locked)
        {
            m_gizmo_data.gizmo_matrix = GfMatrix4d().SetRotate(get_delta()) * m_start_matrix;
            m_gizmo_data.gizmo_matrix = m_gizmo_data.gizmo_matrix.RemoveScaleShear();
        }
    }
}

void ViewportRotateManipulator::on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                                 ViewportUiDrawManager* draw_manager)
{
    m_rotate_mode = RotateMode::NONE;
}

void ViewportRotateManipulator::draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view || m_gizmo_data.gizmo_matrix == GfMatrix4d(0.0))
        return;

    GfCamera camera = viewport_view->get_camera();
    GfFrustum frustum = camera.GetFrustum();

    const auto gizmo_center = m_gizmo_data.gizmo_matrix.ExtractTranslation();
    const double screen_factor = compute_screen_factor(viewport_view, gizmo_center);

    const auto viewport_dim = viewport_view->get_viewport_dimensions();
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit,
                            viewport_dim.height != 0.0 ? (double)viewport_dim.width / viewport_dim.height : 1.0);

    if (GfIsClose(gizmo_center, frustum.GetPosition(), 0.00001))
        return;

    GfMatrix4d proj_matrix = frustum.ComputeProjectionMatrix();
    GfMatrix4d vp_matrix = GfMatrix4d().SetScale(screen_factor) * GfMatrix4d().SetTranslate(m_gizmo_data.gizmo_matrix.ExtractTranslation()) *
                           frustum.ComputeViewMatrix() * proj_matrix;

    GfMatrix4f vp_matrixf = GfMatrix4f(vp_matrix);

    if (m_handle_id_to_axis.empty())
        init_handle_ids(draw_manager);

    auto colors = assign_colors(draw_manager->get_current_selection());

    GfVec3f view = GfVec3f(frustum.ComputeViewDirection());
    GfVec3f up = GfVec3f(frustum.ComputeUpVector());
    up.Normalize();

    GfVec3f orig(0, 0, 0);
    GfVec3f axeX(1, 0, 0);
    GfVec3f axeY(0, 1, 0);
    GfVec3f axeZ(0, 0, 1);

    if (m_orientation == Orientation::GIMBAL)
    {
        const auto indices = get_basis_indices_from_rot_order(m_gizmo_data.rotation_order);
        const auto x_rot = GfMatrix3f(GfRotation(GfVec3f::XAxis(), m_gizmo_data.gizmo_angles[0]));
        const auto y_rot = GfMatrix3f(GfRotation(GfVec3f::YAxis(), m_gizmo_data.gizmo_angles[1]));
        const auto z_rot = GfMatrix3f(GfRotation(GfVec3f::ZAxis(), m_gizmo_data.gizmo_angles[2]));
        const auto parent_rotation = GfMatrix3f(m_gizmo_data.parent_gizmo_matrix.GetOrthonormalized().ExtractRotationMatrix());

        const std::array<GfMatrix3f, 3> rotations = { x_rot, y_rot, z_rot };
        axeX = axeX * get_axis_transform(0, parent_rotation, rotations, indices);
        axeY = axeY * get_axis_transform(1, parent_rotation, rotations, indices);
        axeZ = axeZ * get_axis_transform(2, parent_rotation, rotations, indices);
    }
    else
    {
        axeX = GfVec3f(m_gizmo_data.gizmo_matrix.TransformDir(axeX));
        axeY = GfVec3f(m_gizmo_data.gizmo_matrix.TransformDir(axeY));
        axeZ = GfVec3f(m_gizmo_data.gizmo_matrix.TransformDir(axeZ));
    }
    axeX.Normalize();
    axeY.Normalize();
    axeZ.Normalize();

    GfPlane camera_plane(view, orig);
    GfVec3f right, frnt;
    view.Normalize();

    right = up ^ view;
    right.Normalize();

    draw_utils::draw_outlined_circle(draw_manager, vp_matrixf, *colors[RotateMode::XYZ].transparent, *colors[RotateMode::XYZ].color, orig, right, up,
                                     1, 0, m_axis_to_handle_id[RotateMode::XYZ]);
    if (m_orientation != Orientation::GIMBAL)
        draw_utils::draw_circle(draw_manager, GfMatrix4f().SetScale(1.15) * vp_matrixf, *colors[RotateMode::VIEW].color, orig, right, up, 1, 0,
                                m_axis_to_handle_id[RotateMode::VIEW]);

    right = view ^ axeX;
    right.Normalize();
    frnt = right ^ axeX;
    frnt.Normalize();
    draw_utils::draw_circle_half(draw_manager, vp_matrixf, *colors[RotateMode::X].color, orig, right, frnt, camera_plane, 1,
                                 m_axis_to_handle_id[RotateMode::X]);

    right = view ^ axeY;
    right.Normalize();
    frnt = right ^ axeY;
    frnt.Normalize();
    draw_utils::draw_circle_half(draw_manager, vp_matrixf, *colors[RotateMode::Y].color, orig, right, frnt, camera_plane, 1,
                                 m_axis_to_handle_id[RotateMode::Y]);

    right = view ^ axeZ;
    right.Normalize();
    frnt = right ^ axeZ;
    frnt.Normalize();
    draw_utils::draw_circle_half(draw_manager, vp_matrixf, *colors[RotateMode::Z].color, orig, right, frnt, camera_plane, 1,
                                 m_axis_to_handle_id[RotateMode::Z]);

    switch (m_rotate_mode)
    {
    case RotateMode::X:
        draw_pie(draw_manager, vp_matrixf, 1.0, orig, axeX);
        break;
    case RotateMode::Y:
        draw_pie(draw_manager, vp_matrixf, 1.0, orig, axeY);
        break;
    case RotateMode::Z:
        draw_pie(draw_manager, vp_matrixf, 1.0, orig, axeZ);
        break;
    case RotateMode::VIEW:
        draw_pie(draw_manager, vp_matrixf, 1.15, orig, -view);
        break;
    }
}

bool ViewportRotateManipulator::is_locked() const
{
    return m_is_locked;
}

void ViewportRotateManipulator::set_locked(bool locked)
{
    m_is_locked = locked;
}

void ViewportRotateManipulator::set_orientation(Orientation orientation)
{
    m_orientation = orientation;
}

bool ViewportRotateManipulator::is_gizmo_locked() const
{
    return m_is_gizmo_locked;
}

void ViewportRotateManipulator::set_gizmo_locked(bool locked)
{
    m_is_gizmo_locked = locked;
}

ViewportRotateManipulator::Orientation ViewportRotateManipulator::get_orientation() const
{
    return m_orientation;
}

ViewportRotateManipulator::RotateMode ViewportRotateManipulator::get_rotate_mode() const
{
    return m_rotate_mode;
}

GfRotation ViewportRotateManipulator::get_delta() const
{
    if (!m_is_step_mode_enabled)
        return m_delta;

    GfRotation result = m_delta;
    const auto div = result.GetAngle() / m_step;
    const auto integer = static_cast<int64_t>(div);
    result.SetAxisAngle(result.GetAxis(), m_step * integer + m_step * std::round(div - integer));
    return result;
}

void ViewportRotateManipulator::set_gizmo_data(const GizmoData& gizmo_data)
{
    m_gizmo_data = gizmo_data;
    m_start_matrix = gizmo_data.gizmo_matrix;
    m_inv_start_matrix = m_start_matrix.GetInverse();
    m_delta = GfRotation().SetIdentity();
}

bool ViewportRotateManipulator::is_step_mode_enabled() const
{
    return m_is_step_mode_enabled;
}

void ViewportRotateManipulator::enable_step_mode(bool enable)
{
    m_is_step_mode_enabled = enable;
}

double ViewportRotateManipulator::get_step() const
{
    return m_step;
}

void ViewportRotateManipulator::set_step(double step)
{
    if (GfIsClose(step, 0, 0.000001))
        step = 10.0;
    m_step = std::abs(step);
}

bool ViewportRotateManipulator::is_valid() const
{
    return GfMatrix4d().SetZero() != m_start_matrix;
}

ViewportRotateManipulator::GizmoColors ViewportRotateManipulator::assign_colors(uint32_t selected_handle)
{
    static GizmoColors locked_colors;
    static std::unordered_map<RotateMode, GizmoColors> colors_per_mode;
    static std::once_flag once;
    std::call_once(once, [] {
        for (int i = 0; i < static_cast<int>(RotateMode::COUNT); i++)
        {
            auto mode = static_cast<RotateMode>(i);
            locked_colors[mode] = { &g_lock_color, &g_lock_color_transparent };
            GizmoColors colors;
            switch (mode)
            {
            case RotateMode::NONE:
            {
                colors[RotateMode::X].color = &g_x_color;
                colors[RotateMode::Y].color = &g_y_color;
                colors[RotateMode::Z].color = &g_z_color;
                colors[RotateMode::VIEW].color = &g_view_color;
                colors[RotateMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            }
            case RotateMode::X:
            {
                colors[RotateMode::X].color = &g_select_color;
                colors[RotateMode::Y].color = &g_y_color;
                colors[RotateMode::Z].color = &g_z_color;
                colors[RotateMode::VIEW].color = &g_view_color;
                colors[RotateMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            }
            case RotateMode::Y:
            {
                colors[RotateMode::X].color = &g_x_color;
                colors[RotateMode::Y].color = &g_select_color;
                colors[RotateMode::Z].color = &g_z_color;
                colors[RotateMode::VIEW].color = &g_view_color;
                colors[RotateMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            }
            case RotateMode::Z:
            {
                colors[RotateMode::X].color = &g_x_color;
                colors[RotateMode::Y].color = &g_y_color;
                colors[RotateMode::Z].color = &g_select_color;
                colors[RotateMode::VIEW].color = &g_view_color;
                colors[RotateMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            }
            case RotateMode::VIEW:
            {
                colors[RotateMode::X].color = &g_x_color;
                colors[RotateMode::Y].color = &g_y_color;
                colors[RotateMode::Z].color = &g_z_color;
                colors[RotateMode::VIEW].color = &g_select_color;
                colors[RotateMode::XYZ] = { &g_xyz_color, &g_xyz_color_transparent };
                break;
            }
            case RotateMode::XYZ:
            {
                colors[RotateMode::X].color = &g_x_color;
                colors[RotateMode::Y].color = &g_y_color;
                colors[RotateMode::Z].color = &g_z_color;
                colors[RotateMode::VIEW].color = &g_view_color;
                colors[RotateMode::XYZ] = { &g_xyz_color, &g_xyz_locate_color_transparent };
                break;
            }
            }
            colors_per_mode[mode] = colors;
        }
    });

    if (is_locked())
        return locked_colors;

    auto result = colors_per_mode[m_rotate_mode];
    if (m_rotate_mode == RotateMode::NONE)
    {
        auto iter = m_handle_id_to_axis.find(selected_handle);
        if (iter != m_handle_id_to_axis.end())
            result[iter->second] = { iter->second == RotateMode::XYZ ? &g_xyz_color : &g_locate_color,
                                     iter->second == RotateMode::XYZ ? &g_xyz_locate_color_transparent : &g_locate_color };
    }

    return result;
}

void ViewportRotateManipulator::init_handle_ids(ViewportUiDrawManager* draw_manager)
{
    if (!draw_manager)
        return;

    for (int mode = 1; mode < static_cast<int>(RotateMode::COUNT); mode++)
    {
        auto insert_result = m_axis_to_handle_id.emplace(static_cast<RotateMode>(mode), draw_manager->create_selection_id());
        m_handle_id_to_axis[insert_result.first->second] = static_cast<RotateMode>(mode);
    }
}

void ViewportRotateManipulator::draw_pie(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& vp_matrix, double radius,
                                         const PXR_NS::GfVec3f& orig, const PXR_NS::GfVec3f& axis) const
{
    auto v1_proj = GfVec3f(m_start_vector) - GfDot(m_start_vector, axis) * axis;
    v1_proj.Normalize();

    auto v2_proj = GfVec3f(GfRotation(axis, get_delta().GetAngle()).TransformDir(v1_proj));
    v2_proj.Normalize();
    if (GfDot(GfCross(v2_proj, v1_proj), axis) < 0)
        std::swap(v1_proj, v2_proj);

    v1_proj *= radius;
    v2_proj *= radius;

    // draw arc
    draw_manager->begin_drawable();
    draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeTriangleFan);
    draw_manager->set_mvp_matrix(vp_matrix);
    draw_manager->set_depth_priority(2);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::STIPPLED);
    draw_manager->set_color(g_pie_color);
    draw_manager->arc(orig, v1_proj, v2_proj, axis, radius, true);
    draw_manager->end_drawable();

    // draw start/end vectors
    draw_manager->begin_drawable();
    draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLines);
    draw_manager->set_color(g_pie_color);
    draw_manager->set_mvp_matrix(vp_matrix);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
    draw_manager->line(orig, GfVec3f(v1_proj));
    draw_manager->line(orig, GfVec3f(v2_proj));
    draw_manager->set_line_width(1);
    draw_manager->set_depth_priority(2);
    draw_manager->end_drawable();

    // draw points
    const GfVec3f v1_screen_pos = vp_matrix.Transform(GfVec3f(v1_proj));
    const GfVec3f v2_screen_pos = vp_matrix.Transform(GfVec3f(v2_proj));
    const GfVec3f orig_screen_pos = vp_matrix.Transform(GfVec3f(orig));
    draw_manager->begin_drawable();
    draw_manager->set_color(g_pie_color);
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::FLAT);
    draw_manager->set_depth_priority(2);
    draw_manager->set_point_size(8.0f);
    draw_manager->mesh(ViewportUiDrawManager::PrimitiveType::PrimitiveTypePoints, { v1_screen_pos, v2_screen_pos, orig_screen_pos });
    draw_manager->end_drawable();
}

OPENDCC_NAMESPACE_CLOSE
