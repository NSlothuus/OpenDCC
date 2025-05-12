/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/application.h"
#include "opendcc/usd_editor/common_tools/viewport_select_tool_context.h"
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usdGeom/xformable.h>
#include "opendcc/app/core/session.h"
#include "opendcc/app/viewport/viewport_scale_manipulator.h"

class QMouseEvent;

namespace PXR_NS
{
    TF_DECLARE_PUBLIC_TOKENS(ScaleToolTokens, ((name, "scale_tool")));
};

OPENDCC_NAMESPACE_OPEN
class ViewportGLWidget;
class ScaleToolCommand;
class ViewportScaleToolCommand;
class ViewportPivotEditor;

class ViewportScaleToolContext : public ViewportSelectToolContext
{
    Q_OBJECT
public:
    using StepMode = ViewportScaleManipulator::StepMode;
    ViewportScaleToolContext();
    virtual ~ViewportScaleToolContext();

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
    void set_edit_pivot(bool is_edit);
    void reset_pivot();
    StepMode get_step_mode() const;
    void set_step_mode(StepMode mode);
    double get_step() const;
    void set_step(double step);

Q_SIGNALS:
    void edit_pivot_mode_enabled(bool);

private:
    void update_gizmo_via_selection();

    static bool s_factory_registration;
    std::unique_ptr<ViewportScaleManipulator> m_manipulator;
    std::shared_ptr<ViewportScaleToolCommand> m_scale_command;
    Application::CallbackHandle m_selection_changed_id;
    Application::CallbackHandle m_time_changed_id;
    Session::StageChangedCallbackHandle m_stage_object_changed_id;
    std::unique_ptr<ViewportPivotEditor> m_pivot_editor;
    unsigned long long m_key_press_timepoint = -1;
    bool m_edit_pivot = false;
};

OPENDCC_NAMESPACE_CLOSE
