/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include "opendcc/usd_editor/common_tools/viewport_select_tool_context.h"
#include "opendcc/app/viewport/viewport_move_manipulator.h"
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usdGeom/xformable.h>
#include <pxr/usd/usdGeom/pointBased.h>
#include <QCursor>

namespace PXR_NS
{
    TF_DECLARE_PUBLIC_TOKENS(MoveToolTokens, ((name, "move_tool")));
};

class QMouseEvent;

OPENDCC_NAMESPACE_OPEN

class ViewportGLWidget;
class ViewportMoveManipulator;
class ViewportMoveToolCommand;
class ViewportPivotEditor;

class ViewportMoveToolContext : public ViewportSelectToolContext
{
    Q_OBJECT
public:
    enum class AxisOrientation
    {
        OBJECT,
        WORLD,
        COUNT
    };

    enum class SnapMode
    {
        OFF,
        RELATIVE_MODE,
        ABSOLUTE_MODE,
        GRID,
        VERTEX,
        EDGE,
        EDGE_CENTER,
        FACE_CENTER,
        OBJECT_SURFACE
    };

    ViewportMoveToolContext();
    virtual ~ViewportMoveToolContext();

    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_press(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual bool on_key_release(QKeyEvent* key_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

    virtual PXR_NS::TfToken get_name() const override;

    void set_axis_orientation(AxisOrientation axis_orientation);
    AxisOrientation get_axis_orientation() const { return m_axis_orientation; }
    void set_edit_pivot(bool is_edit);
    void reset_pivot();
    SnapMode get_snap_mode() const;
    void set_snap_mode(SnapMode mode);
    double get_step() const;
    void set_step(double step);

Q_SIGNALS:
    void edit_pivot_mode_enabled(bool);

private:
    void update_gizmo_via_selection();
    void update_snap_strategy();

    std::unique_ptr<ViewportMoveManipulator> m_manipulator;
    std::shared_ptr<ViewportMoveToolCommand> m_move_command;
    AxisOrientation m_axis_orientation = AxisOrientation::WORLD;
    Application::CallbackHandle m_selection_changed_id;
    Application::CallbackHandle m_time_changed_id;
    Session::StageChangedCallbackHandle m_stage_object_changed_id;
    std::unique_ptr<ViewportPivotEditor> m_pivot_editor;
    std::shared_ptr<ViewportSnapStrategy> m_snap_strategy;
    std::unordered_map<std::string, Settings::SettingChangedHandle> m_settings_changed_cid;
    SnapMode m_snap_mode = SnapMode::OFF;
    unsigned long long m_key_press_timepoint = -1;
    bool m_edit_pivot = false;
};

OPENDCC_NAMESPACE_CLOSE
