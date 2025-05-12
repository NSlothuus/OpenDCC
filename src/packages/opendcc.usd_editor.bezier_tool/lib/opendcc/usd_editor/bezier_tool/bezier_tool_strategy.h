/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include "opendcc/app/viewport/iviewport_tool_context.h"
#include "opendcc/app/viewport/viewport_scale_manipulator.h"
#include "opendcc/app/viewport/viewport_move_manipulator.h"

#include "opendcc/usd_editor/bezier_tool/bezier_curve.h"

#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix4d.h>

#include <functional>

OPENDCC_NAMESPACE_OPEN

class BezierToolContext;

//////////////////////////////////////////////////////////////////////////
// BezierToolStrategy
//////////////////////////////////////////////////////////////////////////

class BezierToolStrategy
{
public:
    BezierToolStrategy(BezierToolContext* context);
    virtual ~BezierToolStrategy();

    /**
     * @brief Checks whether the strategy is completed.
     *
     * @return true if this strategy is completed, otherwise false.
     */
    bool is_finished();

    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) = 0;
    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) = 0;
    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) = 0;

    virtual void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager);

protected:
    BezierToolContext* get_context();
    virtual void set_finished(bool finished);

private:
    BezierToolContext* m_context;
    bool m_finished = false;
};
using BezierToolStrategyPtr = std::shared_ptr<BezierToolStrategy>;

//////////////////////////////////////////////////////////////////////////
// BezierToolNullStrategy
//////////////////////////////////////////////////////////////////////////

class BezierToolNullStrategy final : public BezierToolStrategy
{
public:
    BezierToolNullStrategy(BezierToolContext* context);

    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
};

//////////////////////////////////////////////////////////////////////////
// AddPointStrategy
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Adds a point to the end of a curve on left mouse button click,
 * works only if last point added is selected.
 * If the left mouse button was not released after clicking, the tangents of the new point are edited in Normal mode.
 */
class AddPointStrategy final : public BezierToolStrategy
{
public:
    AddPointStrategy(BezierToolContext* context);

    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

private:
    void update();
    void set_finished(bool finished) override;

    bool m_edit = false;
};

//////////////////////////////////////////////////////////////////////////
// ResetTangentsStrategy
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Resets the tangent values of a point (the tangent coordinates will become equal to the point coordinates)
 * on left mouse button clicking to the point with Ctrl pressed.
 * If the left mouse button is not released after clicking, the tangents of the selected point are edited in Normal mode.
 */
class ResetTangentsStrategy final : public BezierToolStrategy
{
public:
    ResetTangentsStrategy(BezierToolContext* context);

    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

private:
    void update(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view);
    void set_finished(bool finished) override;

    bool m_edit = false;
    BezierCurve::Point m_old_point;

    PXR_NS::GfVec3d m_start_move_point;
    PXR_NS::GfVec3d m_drag_direction;
};

//////////////////////////////////////////////////////////////////////////
// MovePointStrategy
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Moves the selected point by dragging with left mouse button pressed.
 */
class MovePointStrategy final : public BezierToolStrategy
{
public:
    MovePointStrategy(BezierToolContext* context);

    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

private:
    void update(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view);
    void set_finished(bool finished) override;

    bool m_edit = false;
    BezierCurve::Point m_old_point;

    PXR_NS::GfVec3d m_start_move_point;
    PXR_NS::GfVec3d m_drag_direction;
};

//////////////////////////////////////////////////////////////////////////
// EditTangentStrategy
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Edits the tangents of the selected point based on the current tangent Select mode.
 * Editing a tangent with Shift pressed does not affect the angle, only the weight being changed.
 * With Ctrl pressed the selected tangent is edited in Tangent mode, regardless of the current BezierToolContext Select mode.
 */
class EditTangentStrategy final : public BezierToolStrategy
{
public:
    EditTangentStrategy(BezierToolContext* context);

    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

private:
    void update(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view);
    void set_finished(bool finished) override;

    BezierCurve::TangentMode m_mode;
    bool m_edit = false;
    size_t m_point_index = BezierCurve::s_invalid_index;
    BezierCurve::Point m_old_point;

    PXR_NS::GfVec3d m_start_move_point;
    PXR_NS::GfVec3d m_drag_direction;
    PXR_NS::GfVec3d m_plane_point;
};

//////////////////////////////////////////////////////////////////////////
// CloseCurveStrategy
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Sets the curve closed (periodic) if the last point is selected on
 * left mouse click to the first point with Ctrl and Shift pressed.
 */
class CloseCurveStrategy final : public BezierToolStrategy
{
public:
    CloseCurveStrategy(BezierToolContext* context);

    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

private:
    void update(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view);
    void set_finished(bool finished) override;

    bool m_edit = false;
    BezierCurve::Point m_old_point;

    PXR_NS::GfVec3d m_start_move_point;
    PXR_NS::GfVec3d m_drag_direction;
};

//////////////////////////////////////////////////////////////////////////
// GizmoMovePointStrategy
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Draws a Gizmo to move the selected point.
 */
class GizmoMovePointStrategy final : public BezierToolStrategy
{
public:
    GizmoMovePointStrategy(BezierToolContext* context);

    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

    void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

    ViewportMoveManipulator& get_manipulator();

private:
    BezierCurve::Point m_old_point;
    size_t m_point_index = BezierCurve::s_invalid_index;
    bool m_edit = false;
    ViewportMoveManipulator m_manipulator;
};

//////////////////////////////////////////////////////////////////////////
// GizmoScalePointStrategy
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Draws a Gizmo for scaling the tangents of the selected point.
 */
class GizmoScalePointStrategy final : public BezierToolStrategy
{
public:
    GizmoScalePointStrategy(BezierToolContext* context);

    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

    void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

    ViewportScaleManipulator& get_manipulator();

private:
    BezierCurve::Point m_old_point;
    size_t m_point_index = BezierCurve::s_invalid_index;
    bool m_edit = false;
    ViewportScaleManipulator m_manipulator;
    PXR_NS::GfVec3d m_sgn_scale = PXR_NS::GfVec3d(1);
};

OPENDCC_NAMESPACE_CLOSE
