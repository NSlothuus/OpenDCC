/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/iviewport_tool_context.h"

#include "opendcc/usd_editor/sculpt_tool/mesh_manipulation_data.h"
#include "opendcc/usd_editor/sculpt_tool/sculpt_properties.h"

OPENDCC_NAMESPACE_OPEN

// SculptStrategy

class SculptStrategy
{
public:
    SculptStrategy();
    virtual ~SculptStrategy();

    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) = 0;
    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) = 0;
    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) = 0;

    void set_mesh_data(std::shared_ptr<UndoMeshManipulationData> data);

    void set_properties(const Properties& properties);

    const Properties& get_properties() const;

    const PXR_NS::GfVec3f& get_draw_normal() const;

    const PXR_NS::GfVec3f& get_draw_point() const;

    const bool is_intersect() const;

protected:
    void solve_hit_info_by_mesh(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view);

    void do_sculpt(const PXR_NS::VtVec3fArray& prev_points, const std::vector<int>& indices);

    bool m_first_moving = true;
    bool m_is_intersect = false;
    bool m_is_inverts = false;

    PXR_NS::GfVec3f m_hit_normal;
    PXR_NS::GfVec3f m_hit_point;
    PXR_NS::GfVec3f m_direction;

    PXR_NS::GfVec3f m_draw_normal;
    PXR_NS::GfVec3f m_draw_point;

    Properties m_properties;

    std::shared_ptr<UndoMeshManipulationData> m_mesh_data;
};

class DefaultStrategy : public SculptStrategy
{
public:
    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;

    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;

    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
};

class MoveStrategy : public SculptStrategy
{
public:
    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;

    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;

    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;

private:
    void solve_hit_info(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view);

    std::vector<int> m_indices;
    PXR_NS::VtVec3fArray m_prev_points;
    bool m_clicked = false;

    PXR_NS::GfVec3f m_plane_normal;
    PXR_NS::GfVec3f m_plane_point;
};

OPENDCC_NAMESPACE_CLOSE
