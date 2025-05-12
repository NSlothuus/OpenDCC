/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_view.h"
#include "opendcc/usd_editor/common_tools/viewport_change_pivot_command.h"
#include "opendcc/app/viewport/viewport_move_manipulator.h"

class QMouseEvent;

OPENDCC_NAMESPACE_OPEN

class ViewportUiDrawManager;

class ViewportPivotEditor
{
public:
    enum class Orientation
    {
        OBJECT,
        WORLD
    };

    ViewportPivotEditor(const SelectionList& selection_list, Orientation orientation);
    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager);
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager);
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager);
    void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager);
    bool is_editing() const;
    void set_orientation(Orientation orientation);
    void set_snap_strategy(std::shared_ptr<ViewportSnapStrategy> snap_strategy);

private:
    std::unique_ptr<ViewportMoveManipulator> m_manipulator;
    std::shared_ptr<ViewportChangePivotCommand> m_command;
    SelectionList m_selection;
};
OPENDCC_NAMESPACE_CLOSE
