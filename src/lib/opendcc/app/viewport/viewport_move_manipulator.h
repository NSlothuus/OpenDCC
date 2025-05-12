/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/base/gf/matrix4d.h>
#include "opendcc/app/viewport/viewport_view.h"
#include "opendcc/app/viewport/viewport_move_snap_strategy.h"
#include "opendcc/app/viewport/viewport_manipulator.h"
#include <unordered_map>

class QMouseEvent;

OPENDCC_NAMESPACE_OPEN
class ViewportUiDrawManager;
class OPENDCC_API ViewportMoveManipulator : public IViewportManipulator
{
public:
    enum class MoveMode
    {
        NONE,
        X,
        Y,
        Z,
        XY,
        XZ,
        YZ,
        XYZ,
        COUNT
    };

    ViewportMoveManipulator() = default;
    virtual void on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;
    virtual void on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;
    virtual void on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
    virtual void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual bool is_picked() const override { return m_move_mode != MoveMode::NONE; }
    const PXR_NS::GfMatrix4d& get_gizmo_matrix() const;
    MoveMode get_move_mode() const { return m_move_mode; }
    PXR_NS::GfVec3d get_delta() const;
    void set_gizmo_matrix(const PXR_NS::GfMatrix4d& gizmo_matrix);
    bool is_locked() const;
    void set_locked(bool locked);
    void set_snap_strategy(std::shared_ptr<ViewportSnapStrategy> snap_strategy);
    bool is_valid() const;

private:
    struct ColorPair
    {
        const PXR_NS::GfVec4f* color = nullptr;
        const PXR_NS::GfVec4f* transparent = nullptr;
    };
    using GizmoColors = std::unordered_map<MoveMode, ColorPair>;
    GizmoColors assign_colors(uint32_t selected_handle);
    void init_handle_ids(ViewportUiDrawManager* draw_manager);

    PXR_NS::GfMatrix4d m_gizmo_matrix;
    PXR_NS::GfVec3d m_drag_direction;
    PXR_NS::GfVec3d m_drag_plane_translation;
    PXR_NS::GfMatrix4d m_view_projection;
    PXR_NS::GfVec3d m_start_drag_point;
    PXR_NS::GfVec3d m_delta;
    std::function<bool(const ViewportViewPtr&, const PXR_NS::GfVec3d&, const PXR_NS::GfVec3d&, const PXR_NS::GfMatrix4d&, int, int, PXR_NS::GfVec3d&)>
        m_compute_intersection_point;
    std::unordered_map<uint32_t, MoveMode> m_handle_id_to_axis;
    std::unordered_map<MoveMode, uint32_t> m_axis_to_handle_id;
    std::shared_ptr<ViewportSnapStrategy> m_snap_strategy;
    MoveMode m_move_mode = MoveMode::NONE;
    bool m_is_locked = false;
};
OPENDCC_NAMESPACE_CLOSE
