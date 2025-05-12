/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

class QMouseEvent;

OPENDCC_NAMESPACE_OPEN

class UVEditorGLWidget;
class ViewportUiDrawManager;

class UvTool
{
public:
    UvTool(UVEditorGLWidget* widget);
    virtual ~UvTool();

    virtual bool on_mouse_press(QMouseEvent* event) = 0;
    virtual bool on_mouse_move(QMouseEvent* event) = 0;
    virtual bool on_mouse_release(QMouseEvent* event) = 0;

    virtual void draw(ViewportUiDrawManager* draw_manager) = 0;

    virtual bool is_working() const = 0;

    UVEditorGLWidget* get_widget();
    const UVEditorGLWidget* get_widget() const;

protected:
    static bool working_application_selection_mode();

private:
    UVEditorGLWidget* m_widget = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
