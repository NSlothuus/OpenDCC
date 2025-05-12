// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/uv_editor/uv_rotate_tool.h"
#include "opendcc/usd_editor/uv_editor/uv_editor_gl_widget.h"
#include "opendcc/usd_editor/uv_editor/utils.h"

#include "opendcc/base/commands_api/core/command_interface.h"
#include "opendcc/base/commands_api/core/command_registry.h"

#include "opendcc/app/viewport/viewport_camera_controller.h"
#include "opendcc/app/viewport/viewport_widget.h"

#include "opendcc/ui/common_widgets/round_marking_menu.h"

#include <pxr/base/gf/vec4f.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

//////////////////////////////////////////////////////////////////////////
// UvRotateTool
//////////////////////////////////////////////////////////////////////////

UvRotateTool::UvRotateTool(UVEditorGLWidget* widget)
    : UvSelectTool(widget)
    , m_manipulator(this)
{
    update_command();

    auto& app = Application::instance();

    m_selection_changed_id = app.register_event_callback(Application::EventType::SELECTION_CHANGED, [&] { update_command(); });
    m_current_viewport_tool_changed_id =
        app.register_event_callback(Application::EventType::CURRENT_VIEWPORT_TOOL_CHANGED, [&] { update_command(); });
    m_current_stage_object_changed_id = app.get_session()->register_stage_changed_callback(
        Session::StageChangedEventType::CURRENT_STAGE_OBJECT_CHANGED, [&](UsdNotice::ObjectsChanged const& notice) { update_command(); });
}

UvRotateTool::~UvRotateTool()
{
    auto& app = Application::instance();

    app.unregister_event_callback(Application::EventType::SELECTION_CHANGED, m_selection_changed_id);
    app.unregister_event_callback(Application::EventType::CURRENT_VIEWPORT_TOOL_CHANGED, m_current_viewport_tool_changed_id);
    app.get_session()->unregister_stage_changed_callback(Session::StageChangedEventType::CURRENT_STAGE_OBJECT_CHANGED,
                                                         m_current_stage_object_changed_id);
}

bool UvRotateTool::on_mouse_press(QMouseEvent* event) /* override */
{
    switch (current_mode())
    {
    case Mode::Select:
    {
        return UvSelectTool::on_mouse_press(event);
    }
    case Mode::Move:
    case Mode::None:
    default:
    {
        m_manipulator.on_mouse_press(event);
        if (m_manipulator.move_started())
        {
            m_command->start();
            return true;
        }
        else
        {
            return UvSelectTool::on_mouse_press(event);
        }
    }
    }
}

bool UvRotateTool::on_mouse_move(QMouseEvent* event) /* override */
{
    switch (current_mode())
    {
    case Mode::Select:
    {
        return UvSelectTool::on_mouse_move(event);
    }
    case Mode::Move:
    case Mode::None:
    default:
    {
        if (m_manipulator.move_started())
        {
            m_manipulator.on_mouse_move(event);
            m_command->apply_rotate(m_manipulator.get_angle());
            return true;
        }
        else
        {
            return UvSelectTool::on_mouse_move(event);
        }
    }
    }
}

bool UvRotateTool::on_mouse_release(QMouseEvent* event) /* override */
{
    switch (current_mode())
    {
    case Mode::Select:
    {
        return UvSelectTool::on_mouse_release(event);
    }
    case Mode::Move:
    case Mode::None:
    default:
    {
        if (m_manipulator.move_started())
        {
            m_manipulator.on_mouse_release(event);
            m_command->end();
            CommandInterface::finalize(m_command);
            m_command = nullptr;
            update_command();
            return true;
        }
        else
        {
            return UvSelectTool::on_mouse_release(event);
        }
    }
    }
}

void UvRotateTool::draw(ViewportUiDrawManager* draw_manager)
{
    switch (current_mode())
    {
    case Mode::Move:
    {
        m_manipulator.draw(draw_manager);
        break;
    }
    case Mode::Select:
    {
        UvSelectTool::draw(draw_manager);
    }
    case Mode::None:
    default:
    {
        auto& app = Application::instance();
        const auto selection = app.get_selection();
        if (working_application_selection_mode() && !selection.empty())
        {
            m_manipulator.draw(draw_manager);
        }
    }
    }
}

bool UvRotateTool::is_working() const
{
    return UvSelectTool::is_working() || current_mode() != Mode::None;
}

UvRotateTool::Mode UvRotateTool::current_mode() const
{
    const auto select = UvSelectTool::is_working();
    const auto move = m_manipulator.move_started() && m_command && m_command->is_started();

    if (select && !move)
    {
        return UvRotateTool::Mode::Select;
    }
    else if (move)
    {
        return UvRotateTool::Mode::Move;
    }
    else
    {
        return UvRotateTool::Mode::None;
    }
}

void UvRotateTool::update_command()
{
    if (m_command && m_command->is_started())
    {
        return;
    }

    auto& app = Application::instance();
    const auto mode = app.get_selection_mode();
    const auto widget = get_widget();

    m_command = CommandRegistry::create_command<UvRotateCommand>("uv_rotate");
    if (mode == Application::SelectionMode::UV)
    {
        m_command->init_from_uv_selection(widget, widget->get_uv_selection());
    }
    else
    {
        m_command->init_from_mesh_selection(widget, app.get_selection());
    }

    m_manipulator.set_pos(m_command->get_centroid());
}

OPENDCC_NAMESPACE_CLOSE
