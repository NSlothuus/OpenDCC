// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/uv_editor/uv_tool.h"

#include "opendcc/usd_editor/uv_editor/uv_editor_gl_widget.h"

#include "opendcc/app/core/application.h"

OPENDCC_NAMESPACE_OPEN

//////////////////////////////////////////////////////////////////////////
// UvTool
//////////////////////////////////////////////////////////////////////////

UvTool::UvTool(UVEditorGLWidget* widget)
    : m_widget(widget)
{
}

/* virtual */
UvTool::~UvTool() {}

UVEditorGLWidget* UvTool::get_widget()
{
    return m_widget;
}

const UVEditorGLWidget* UvTool::get_widget() const
{
    return m_widget;
}

/* static */
bool UvTool::working_application_selection_mode()
{
    const auto& app = Application::instance();
    const auto mode = app.get_selection_mode();

    return mode == Application::SelectionMode::POINTS || mode == Application::SelectionMode::UV || mode == Application::SelectionMode::EDGES ||
           mode == Application::SelectionMode::FACES;
}

OPENDCC_NAMESPACE_CLOSE
