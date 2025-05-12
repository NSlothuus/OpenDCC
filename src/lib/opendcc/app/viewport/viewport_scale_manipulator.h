/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/base/gf/matrix4d.h>
#include "opendcc/app/viewport/viewport_view.h"
#include "opendcc/app/viewport/viewport_manipulator.h"
#include <unordered_map>

class QMouseEvent;

OPENDCC_NAMESPACE_OPEN

class ViewportUiDrawManager;

class OPENDCC_API ViewportScaleManipulator : public IViewportManipulator
{
public:
    enum class ScaleMode
    {
        NONE = 0,
        X,
        Y,
        Z,
        XY,
        XZ,
        YZ,
        XYZ,
        COUNT
    };
    enum class StepMode
    {
        OFF,
        RELATIVE_MODE,
        ABSOLUTE_MODE
    };
    struct GizmoData
    {
        PXR_NS::GfMatrix4d gizmo_matrix;
        PXR_NS::GfVec3d scale;
    };
    ViewportScaleManipulator() = default;
    virtual void on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;
    virtual void on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;
    virtual void on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
    virtual void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual bool is_picked() const override { return m_scale_mode != ScaleMode::NONE; }
    PXR_NS::GfVec3f get_delta() const;
    bool is_locked() const;
    void set_locked(bool locked);
    void set_gizmo_data(const GizmoData& gizmo_data);
    ScaleMode get_scale_mode() const;
    StepMode get_step_mode() const;
    void set_step_mode(StepMode mode);
    double get_step() const;
    void set_step(double step);
    bool is_valid() const;

private:
    struct ColorPair
    {
        const PXR_NS::GfVec4f* color = nullptr;
        const PXR_NS::GfVec4f* transparent = nullptr;
    };
    using GizmoColors = std::unordered_map<ScaleMode, ColorPair>;
    GizmoColors assign_colors(uint32_t selected_handle);
    void init_handle_ids(ViewportUiDrawManager* draw_manager);
    void draw_axis(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& mvp, const PXR_NS::GfMatrix4f& model_matrix,
                   const PXR_NS::GfVec4f& color, const PXR_NS::GfVec3f& axis, uint32_t axis_id);

    PXR_NS::GfVec3d m_start_drag_point;
    PXR_NS::GfVec3d m_drag_direction;
    PXR_NS::GfVec3f m_delta;
    GizmoData m_gizmo_data;
    PXR_NS::GfMatrix4d m_view_projection;
    PXR_NS::GfMatrix4d m_inv_gizmo_matrix;
    double m_step = 1.0;
    std::function<bool(const ViewportViewPtr&, const PXR_NS::GfVec3d&, const PXR_NS::GfVec3d&, const PXR_NS::GfMatrix4d&, int, int, PXR_NS::GfVec3d&)>
        m_compute_intersection_point;
    std::unordered_map<uint32_t, ScaleMode> m_handle_id_to_axis;
    std::unordered_map<ScaleMode, uint32_t> m_axis_to_handle_id;
    StepMode m_step_mode = StepMode::OFF;
    ScaleMode m_scale_mode = ScaleMode::NONE;
    bool m_is_locked = false;
};
OPENDCC_NAMESPACE_CLOSE
