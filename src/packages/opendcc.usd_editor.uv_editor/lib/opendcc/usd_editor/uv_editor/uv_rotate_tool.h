/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include "opendcc/usd_editor/uv_editor/uv_select_tool.h"
#include "opendcc/usd_editor/uv_editor/uv_rotate_manipulator.h"
#include "opendcc/usd_editor/uv_editor/uv_rotate_command.h"

#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"

#include <memory>

#include <QMouseEvent>

OPENDCC_NAMESPACE_OPEN

class UvRotateTool final : public UvSelectTool
{
public:
    UvRotateTool(UVEditorGLWidget* widget);
    ~UvRotateTool();

    bool on_mouse_press(QMouseEvent* event) override;
    bool on_mouse_move(QMouseEvent* event) override;
    bool on_mouse_release(QMouseEvent* event) override;

    void draw(ViewportUiDrawManager* draw_manager) override;

    bool is_working() const override;

private:
    enum class Mode
    {
        Move,
        Select,
        None,
    };
    Mode current_mode() const;

    void update_command();

    UvRotateManipulator m_manipulator;

    std::shared_ptr<UvRotateCommand> m_command;

    Application::CallbackHandle m_selection_changed_id;
    Application::CallbackHandle m_current_viewport_tool_changed_id;
    Session::StageChangedCallbackHandle m_current_stage_object_changed_id;
};

OPENDCC_NAMESPACE_CLOSE
