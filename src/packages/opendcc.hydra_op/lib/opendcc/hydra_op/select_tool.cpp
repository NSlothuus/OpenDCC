// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/select_tool.h"
#include <QMouseEvent>
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include "opendcc/hydra_op/session.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_PRIVATE_TOKENS(HydraOpSelectToolContextTokens, (SelectTool));

namespace
{
    SelectionList::SelectionMask convert_to_selection_mask(Application::SelectionMode selection_mode)
    {
        using SelectionMode = Application::SelectionMode;
        using SelectionFlags = SelectionList::SelectionFlags;
        switch (selection_mode)
        {
        case SelectionMode::POINTS:
        case SelectionMode::UV:
        case SelectionMode::EDGES:
        case SelectionMode::FACES:
        case SelectionMode::PRIMS:
        case SelectionMode::INSTANCES:
            return SelectionFlags::FULL_SELECTION;
        default:
            return SelectionFlags::ALL;
        }
    }
};

HydraOpSelectToolContext::HydraOpSelectToolContext()
    : m_select_rect_mode(false)
    , m_mousex(0)
    , m_mousey(0)
    , m_start_posx(0)
    , m_start_posy(0)
    , m_shift(false)
{
}

bool HydraOpSelectToolContext::on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                              ViewportUiDrawManager* draw_manager)
{
    Qt::MouseButton button = mouse_event.button();
    m_shift = mouse_event.modifiers() & Qt::ShiftModifier;
    m_ctrl = mouse_event.modifiers() & Qt::ControlModifier;
    if (button == Qt::LeftButton)
    {
        m_start_posx = mouse_event.x();
        m_start_posy = mouse_event.y();
        m_mousex = mouse_event.x();
        m_mousey = mouse_event.y();
        m_select_rect_mode = true;
        return true;
    }
    return false;
}

bool HydraOpSelectToolContext::on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                             ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view)
        return false;

    m_mousex = mouse_event.x();
    m_mousey = mouse_event.y();
    return false;
}

bool HydraOpSelectToolContext::on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                                ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view)
        return false;

    if (m_select_rect_mode)
    {
        m_mousex = mouse_event.x();
        m_mousey = mouse_event.y();
        m_select_rect_mode = false;
        SelectionList target_selection;
        auto selection_mask = convert_to_selection_mask(Application::instance().get_selection_mode());
        if (std::abs(m_mousex - m_start_posx) > 2 && std::abs(m_mousey - m_start_posy) > 2)
            target_selection = viewport_view->pick_multiple_prims(GfVec2f(m_start_posx, m_start_posy), GfVec2f(m_mousex, m_mousey),
                                                                  selection_mask | SelectionList::SelectionFlags::FULL_SELECTION);
        else
            target_selection =
                viewport_view->pick_single_prim(GfVec2f(m_start_posx, m_start_posy), selection_mask | SelectionList::SelectionFlags::FULL_SELECTION);

        auto get_current_selection = [] {
            return HydraOpSession::instance().get_selection();
        };
        auto set_selection_list = [](const SelectionList& list) {
            HydraOpSession::instance().set_selection(list);
        };

        if (m_shift)
        {
            auto merged_selection = get_current_selection();
            merged_selection.merge(target_selection);
            set_selection_list(merged_selection);
            viewport_view->set_selected(merged_selection);
        }
        else if (m_ctrl)
        {
            auto diff_selection = get_current_selection();
            diff_selection.difference(target_selection, SelectionList::SelectionFlags::FULL_SELECTION);
            set_selection_list(diff_selection);
            viewport_view->set_selected(diff_selection);
        }
        else
        {
            set_selection_list(target_selection);
            viewport_view->set_selected(target_selection);
        }

        m_shift = false;
        return true;
    }
    m_shift = false;
    return false;
}

void HydraOpSelectToolContext::draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    if (m_select_rect_mode && std::abs(m_mousex - m_start_posx) > 2 && std::abs(m_mousey - m_start_posy) > 2)
    {
        const auto viewport_dim = viewport_view->get_viewport_dimensions();
        GfVec2f start(2 * float(m_start_posx) / viewport_dim.width - 1, 1 - 2 * float(m_start_posy) / viewport_dim.height);

        GfVec2f end(2 * float(m_mousex) / viewport_dim.width - 1, 1 - 2 * float(m_mousey) / viewport_dim.height);

        draw_manager->begin_drawable();
        draw_manager->set_color(GfVec4f(1, 1, 1, 1));
        draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::STIPPLED);
        draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLinesStrip);
        draw_manager->rect2d(start, end);
        draw_manager->end_drawable();
    }
}

TfToken HydraOpSelectToolContext::get_name() const
{
    return HydraOpSelectToolContextTokens->SelectTool;
}

bool HydraOpSelectToolContext::is_locked() const
{
    return m_select_rect_mode;
}

OPENDCC_NAMESPACE_CLOSE
