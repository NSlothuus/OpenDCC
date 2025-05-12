/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_view.h"
#include "opendcc/app/viewport/iviewport_tool_context.h"

OPENDCC_NAMESPACE_OPEN
class ViewportUiDrawManager;

class OPENDCC_API IViewportManipulator
{
public:
    virtual void on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) = 0;
    virtual void on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) = 0;
    virtual void on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) = 0;
    virtual void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) = 0;
    virtual bool is_picked() const = 0;
};

OPENDCC_NAMESPACE_CLOSE
