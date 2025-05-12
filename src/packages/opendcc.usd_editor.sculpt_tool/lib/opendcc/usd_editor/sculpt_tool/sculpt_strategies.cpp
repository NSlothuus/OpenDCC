// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/sculpt_tool/sculpt_strategies.h"

#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/camera.h>
#include <pxr/imaging/cameraUtil/conformWindow.h>

#include "opendcc/usd_editor/sculpt_tool/sculpt_functions.h"
#include "opendcc/usd_editor/sculpt_tool/utils.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

// SculptStrategy

SculptStrategy::SculptStrategy()
{
    m_properties.read_from_settings(SculptToolContext::settings_prefix());
}

SculptStrategy::~SculptStrategy() {}

void SculptStrategy::set_mesh_data(std::shared_ptr<UndoMeshManipulationData> data)
{
    m_mesh_data = data;
}

void SculptStrategy::set_properties(const Properties& properties)
{
    m_properties = properties;
}

const Properties& SculptStrategy::get_properties() const
{
    return m_properties;
}

const GfVec3f& SculptStrategy::get_draw_normal() const
{
    return m_draw_normal;
}

const GfVec3f& SculptStrategy::get_draw_point() const
{
    return m_draw_point;
}

const bool SculptStrategy::is_intersect() const
{
    return m_is_intersect;
}

void SculptStrategy::solve_hit_info_by_mesh(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view)
{
    GfVec3f start;
    GfVec3f direction;
    solve_ray_info(mouse_event, viewport_view, start, direction);

    GfVec3f hit_point;
    GfVec3f hit_normal;

    m_is_intersect = m_mesh_data->mesh_bvh.cast_ray(start, direction, hit_point, hit_normal);

    if (m_is_intersect)
    {
        if (m_first_moving)
        {
            m_hit_point = hit_point;
            m_direction = GfVec3f(0);
        }
        else
        {
            m_direction = hit_point - m_hit_point;
            m_hit_point = hit_point;
        }
        m_hit_normal = m_properties.mode == Mode::Move ? -direction.GetNormalized() : hit_normal;
    }

    m_draw_point = m_hit_point;
    m_draw_normal = m_hit_normal;
}

void SculptStrategy::do_sculpt(const VtVec3fArray& prev_points, const std::vector<int>& indices)
{
    if (!m_is_intersect && m_mesh_data && (m_properties.radius > 0.01f))
        return;

    SculptIn sculpt_in;
    sculpt_in.hit_normal = m_hit_normal;
    sculpt_in.hit_point = m_hit_point;
    sculpt_in.direction = m_direction;
    sculpt_in.properties = m_properties;
    sculpt_in.mesh_data = m_mesh_data;
    sculpt_in.inverts = m_is_inverts;

    const VtVec3fArray next_values_vec3f = sculpt(sculpt_in, prev_points, indices);
    if (next_values_vec3f.size() == 0)
        return;
    if (!m_mesh_data->undo_block)
        m_mesh_data->undo_block = std::make_unique<commands::UsdEditsUndoBlock>();
    auto points_attr = m_mesh_data->mesh.GetPointsAttr();
    points_attr.Set(next_values_vec3f);
}

// DefaultStrategy

bool DefaultStrategy::on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    m_first_moving = true;
    on_mouse_move(mouse_event, viewport_view, draw_manager);

    return m_is_intersect;
}

bool DefaultStrategy::on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    solve_hit_info_by_mesh(mouse_event, viewport_view);

    const auto buttons = mouse_event.buttons();
    const auto modifiers = mouse_event.modifiers();

    m_is_inverts = modifiers == Qt::ControlModifier;

    if (buttons != Qt::LeftButton || (modifiers != Qt::NoModifier && modifiers != Qt::ControlModifier))
        return true;

    const std::vector<int> indices = m_mesh_data->mesh_bvh.get_points_in_radius(m_hit_point, m_mesh_data->mesh.GetPath(), m_properties.radius);

    VtVec3fArray prev_points;
    m_mesh_data->mesh.GetPointsAttr().Get(&prev_points);

    do_sculpt(prev_points, indices);
    m_first_moving = false;

    return true;
}

bool DefaultStrategy::on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                       ViewportUiDrawManager* draw_manager)
{
    return true;
}

// MoveStrategy

bool MoveStrategy::on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    if (m_clicked)
        return false;

    m_first_moving = true;

    on_mouse_move(mouse_event, viewport_view, draw_manager);

    if (!m_is_intersect)
        return false;

    m_mesh_data->mesh.GetPointsAttr().Get(&m_prev_points);
    m_indices = m_mesh_data->mesh_bvh.get_points_in_radius(m_hit_point, m_mesh_data->mesh.GetPath(), m_properties.radius);

    m_clicked = true;

    m_plane_normal = m_hit_normal;
    m_plane_point = m_hit_point;

    return true;
}

bool MoveStrategy::on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    solve_hit_info(mouse_event, viewport_view);

    const auto buttons = mouse_event.buttons();
    const auto modifiers = mouse_event.modifiers();

    m_is_inverts = modifiers == Qt::ControlModifier;

    if (buttons != Qt::LeftButton || (modifiers != Qt::NoModifier && modifiers != Qt::ControlModifier))
        return true;

    do_sculpt(m_prev_points, m_indices);
    m_first_moving = false;

    return true;
}

bool MoveStrategy::on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    m_prev_points.clear();
    m_indices.clear();
    m_clicked = false;
    return true;
}

void MoveStrategy::solve_hit_info(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view)
{
    if (m_clicked && m_is_intersect)
    {
        PXR_NS::GfVec3f start;
        PXR_NS::GfVec3f direction;
        solve_ray_info(mouse_event, viewport_view, start, direction);

        const float intersection = line_plane_intersection(direction, start, m_plane_normal, m_plane_point);

        if (intersection == FLT_MAX || intersection < 0)
        {
            m_is_intersect = false;
            return;
        }

        m_is_intersect = true;

        PXR_NS::GfVec3f hit_point = start + intersection * direction;
        PXR_NS::GfVec3f hit_normal = m_plane_normal;

        m_direction = hit_point - m_plane_point;
        m_hit_point = m_plane_point;
        m_hit_normal = m_plane_normal;

        m_draw_point = hit_point;
        m_draw_normal = m_plane_normal;
    }
    else
    {
        solve_hit_info_by_mesh(mouse_event, viewport_view);
    }
}

OPENDCC_NAMESPACE_CLOSE
