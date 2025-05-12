/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/base/gf/matrix4d.h>
#include "opendcc/app/viewport/viewport_view.h"
#include <unordered_map>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include "opendcc/app/viewport/viewport_manipulator.h"

class QMouseEvent;

OPENDCC_NAMESPACE_OPEN

class ViewportUiDrawManager;
class ViewportRotateToolCommand;

class OPENDCC_API ViewportRotateManipulator : public IViewportManipulator
{
public:
    enum class Orientation
    {
        OBJECT,
        WORLD,
        GIMBAL,
        COUNT
    };
    enum class RotateMode
    {
        NONE,
        X,
        Y,
        Z,
        VIEW,
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
        PXR_NS::GfMatrix4d parent_gizmo_matrix;
        PXR_NS::GfVec3f gizmo_angles;
        PXR_NS::UsdGeomXformCommonAPI::RotationOrder rotation_order;
    };

    virtual void on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;
    virtual void on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;
    virtual void on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
    virtual void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual bool is_picked() const override { return m_rotate_mode != RotateMode::NONE; }
    bool is_locked() const;
    void set_locked(bool locked);
    void set_orientation(Orientation orientation);
    bool is_gizmo_locked() const;
    void set_gizmo_locked(bool locked);
    Orientation get_orientation() const;
    RotateMode get_rotate_mode() const;
    PXR_NS::GfRotation get_delta() const;
    void set_gizmo_data(const GizmoData& gizmo_data);
    bool is_step_mode_enabled() const;
    void enable_step_mode(bool enable);
    double get_step() const;
    void set_step(double step);
    bool is_valid() const;

private:
    struct ColorPair
    {
        const PXR_NS::GfVec4f* color = nullptr;
        const PXR_NS::GfVec4f* transparent = nullptr;
    };
    using GizmoColors = std::unordered_map<RotateMode, ColorPair>;
    GizmoColors assign_colors(uint32_t selected_handle);
    void init_handle_ids(ViewportUiDrawManager* draw_manager);
    void draw_pie(ViewportUiDrawManager* draw_manager, const PXR_NS::GfMatrix4f& vp_matrix, double radius, const PXR_NS::GfVec3f& orig,
                  const PXR_NS::GfVec3f& axis) const;

    GizmoData m_gizmo_data;
    PXR_NS::GfVec3f m_start_gizmo_angles;
    PXR_NS::GfVec3f m_axis;
    PXR_NS::GfMatrix4d m_start_matrix;
    PXR_NS::GfMatrix4d m_inv_start_matrix;
    PXR_NS::GfVec3d m_start_vector;
    PXR_NS::GfRotation m_delta;
    double m_step = 10.0;
    bool m_is_step_mode_enabled = false;
    std::unordered_map<uint32_t, RotateMode> m_handle_id_to_axis;
    std::unordered_map<RotateMode, uint32_t> m_axis_to_handle_id;
    RotateMode m_rotate_mode = RotateMode::NONE;
    Orientation m_orientation = Orientation::OBJECT;
    bool m_is_locked = false;
    bool m_is_gizmo_locked = false;
};
OPENDCC_NAMESPACE_CLOSE
