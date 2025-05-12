// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/common_tools/viewport_render_region_tool_context.h"
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include "opendcc/app/ui/application_ui.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include <pxr/imaging/cameraUtil/conformWindow.h>
#include <pxr/base/gf/frustum.h>

namespace PXR_NS
{
    TF_DEFINE_PUBLIC_TOKENS(RenderRegionToolTokens, ((name, "render_region_tool")));
};

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

ViewportRenderRegionToolContext::ViewportRenderRegionToolContext()
    : ViewportSelectToolContext()
    , m_start(ViewportRenderRegionSession::instance().get_start())
    , m_end(ViewportRenderRegionSession::instance().get_end())
    , m_cursor(new QCursor(QPixmap(":/icons/cursor_crosshair")))
{
}

ViewportRenderRegionToolContext::~ViewportRenderRegionToolContext()
{
    delete m_cursor;
}

bool ViewportRenderRegionToolContext::on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                                     ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view)
        return false;

    QPoint mouse_pos(mouse_event.x(), mouse_event.y());

    if (m_selected_pin != RegionPin::None && m_region.isValid())
    {
        m_move_mode = true;
        m_mouse_prev_pos = mouse_pos;
        return true;
    }

    m_selection.moveTo(mouse_pos);
    return ViewportSelectToolContext::on_mouse_press(mouse_event, viewport_view, draw_manager);
}

bool ViewportRenderRegionToolContext::on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                                    ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view)
        return false;

    QPoint mouse_pos(mouse_event.x(), mouse_event.y());
    if (m_move_mode)
    {
        auto diff = mouse_pos - m_mouse_prev_pos;
        static const int min_size = 10;
        switch (m_selected_pin)
        {
        case RegionPin::TopLeft:
        {
            m_region.adjust(diff.x(), diff.y(), 0, 0);
            if (m_region.width() < min_size)
            {
                m_region.setLeft(m_region.right() - min_size);
                mouse_pos.setX(m_region.left());
            }
            if (m_region.height() < min_size)
            {
                m_region.setTop(m_region.bottom() - min_size);
                mouse_pos.setY(m_region.top());
            }
            break;
        }
        case RegionPin::Top:
        {
            m_region.adjust(0, diff.y(), 0, 0);
            if (m_region.height() < min_size)
            {
                m_region.setTop(m_region.bottom() - min_size);
                mouse_pos.setY(m_region.top());
            }
            break;
        }
        case RegionPin::TopRight:
        {
            m_region.adjust(0, diff.y(), diff.x(), 0);
            if (m_region.width() < min_size)
            {
                m_region.setRight(m_region.left() + min_size);
                mouse_pos.setX(m_region.right());
            }
            if (m_region.height() < min_size)
            {
                m_region.setTop(m_region.bottom() - min_size);
                mouse_pos.setY(m_region.top());
            }
            break;
        }
        case RegionPin::Right:
        {
            m_region.adjust(0, 0, diff.x(), 0);
            if (m_region.width() < min_size)
            {
                m_region.setRight(m_region.left() + min_size);
                mouse_pos.setX(m_region.right());
            }
            break;
        }
        case RegionPin::BottomRight:
        {
            m_region.adjust(0, 0, diff.x(), diff.y());
            if (m_region.width() < min_size)
            {
                m_region.setRight(m_region.left() + min_size);
                mouse_pos.setX(m_region.right());
            }
            if (m_region.height() < min_size)
            {
                m_region.setBottom(m_region.top() + min_size);
                mouse_pos.setY(m_region.bottom());
            }
            break;
        }
        case RegionPin::Bottom:
        {
            m_region.adjust(0, 0, 0, diff.y());
            if (m_region.height() < min_size)
            {
                m_region.setBottom(m_region.top() + min_size);
                mouse_pos.setY(m_region.bottom());
            }
            break;
        }
        case RegionPin::BottomLeft:
        {
            m_region.adjust(diff.x(), 0, 0, diff.y());
            if (m_region.width() < min_size)
            {
                m_region.setLeft(m_region.right() - min_size);
                mouse_pos.setX(m_region.left());
            }
            if (m_region.height() < min_size)
            {
                m_region.setBottom(m_region.top() + min_size);
                mouse_pos.setY(m_region.bottom());
            }
            break;
        }
        case RegionPin::Left:
        {
            m_region.adjust(diff.x(), 0, 0, 0);
            if (m_region.width() < min_size)
            {
                m_region.setLeft(m_region.right() - min_size);
                mouse_pos.setX(m_region.left());
            }
            break;
        }
        case RegionPin::Border:
        {
            m_region.adjust(diff.x(), diff.y(), diff.x(), diff.y());
            break;
        }
        default:
            break;
        }

        calc_corners(viewport_view);
        m_mouse_prev_pos = mouse_pos;
        return true;
    }

    if (region_valid())
    {
        auto iter = m_handle_id_to_pin.find(draw_manager->get_current_selection());
        if (iter != m_handle_id_to_pin.end() && m_mouse_inside_region && !m_select_rect_mode)
            m_selected_pin = iter->second;
        else
        {
            m_selected_pin = RegionPin::None;
            static const int mouse_zone = 7;
            if (!m_select_rect_mode)
                m_mouse_inside_region = m_region.adjusted(-mouse_zone, -mouse_zone, mouse_zone, mouse_zone).contains(mouse_pos);
        }
    }
    else
        m_mouse_inside_region = false;

    if (m_select_rect_mode)
        m_selection.setBottomRight(mouse_pos);

    return ViewportSelectToolContext::on_mouse_move(mouse_event, viewport_view, draw_manager);
}

bool ViewportRenderRegionToolContext::on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                                       ViewportUiDrawManager* draw_manager)
{
    if (!viewport_view)
        return false;

    if (m_select_rect_mode && !m_selection.isNull())
    {
        m_region = m_selection.normalized();
        calc_corners(viewport_view);
    }

    m_selection = {};
    m_selected_pin = RegionPin::None;
    m_move_mode = false;
    m_select_rect_mode = false;
    m_mouse_inside_region = true;
    return true;
}

void ViewportRenderRegionToolContext::draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager)
{
    ViewportSelectToolContext::draw(viewport_view, draw_manager);

    if (!region_valid())
        return;

    if (m_handle_id_to_pin.empty())
        init_handle_ids(draw_manager);

    if (!m_move_mode)
        m_region = ViewportRenderRegionSession::instance().get_rect(viewport_view);

    auto viewport_dim = viewport_view->get_viewport_dimensions();
    auto frustum = viewport_view->get_camera().GetFrustum();
    CameraUtilConformWindow(&frustum, CameraUtilConformWindowPolicy::CameraUtilFit,
                            viewport_dim.height != 0 ? (double)viewport_dim.width / viewport_dim.height : 1.0);

    static const GfVec4f main_color(0.4, 0.86, 1, 1);
    static const GfVec4f select_color(0.86, 0.71, 0.49, 1);
    const float pin_horiz_width = 5.f / viewport_dim.height;
    const float pin_vert_width = 5.f / viewport_dim.width;
    const float pin_horiz_length = (m_end[0] - m_start[0]) / 7;
    const float pin_vert_length = (m_end[1] - m_start[1]) / 7;

    auto draw_horiz_pins = [&](bool top) {
        const float y = top ? m_start[1] : m_end[1];
        const float upper_edge = y - pin_horiz_width;
        const float lower_edge = y + pin_horiz_width;

        // left
        RegionPin pin = top ? RegionPin::TopLeft : RegionPin::BottomLeft;
        std::vector<GfVec3f> pin_quad = { GfVec3f(m_start[0] - pin_vert_width, upper_edge, 0), GfVec3f(m_start[0] + pin_horiz_length, upper_edge, 0),
                                          GfVec3f(m_start[0] + pin_horiz_length, lower_edge, 0),
                                          GfVec3f(m_start[0] - pin_vert_width, lower_edge, 0) };
        auto color = m_selected_pin == pin ? select_color : main_color;
        draw_utils::draw_outlined_quad(draw_manager, GfMatrix4f(1), color, color, pin_quad, 0.f, 0, m_pin_to_handle_id[pin]);

        // middle
        pin = top ? RegionPin::Top : RegionPin::Bottom;
        const float middle_x = (m_end[0] + m_start[0]) / 2;
        pin_quad = { GfVec3f(middle_x - pin_horiz_length, upper_edge, 0), GfVec3f(middle_x + pin_horiz_length, upper_edge, 0),
                     GfVec3f(middle_x + pin_horiz_length, lower_edge, 0), GfVec3f(middle_x - pin_horiz_length, lower_edge, 0) };
        color = m_selected_pin == pin ? select_color : main_color;
        draw_utils::draw_outlined_quad(draw_manager, GfMatrix4f(1), color, color, pin_quad, 0.f, 0, m_pin_to_handle_id[pin]);

        // right
        pin = top ? RegionPin::TopRight : RegionPin::BottomRight;
        pin_quad = { GfVec3f(m_end[0] - pin_horiz_length, upper_edge, 0), GfVec3f(m_end[0] + pin_vert_width, upper_edge, 0),
                     GfVec3f(m_end[0] + pin_vert_width, lower_edge, 0), GfVec3f(m_end[0] - pin_horiz_length, lower_edge, 0) };
        color = m_selected_pin == pin ? select_color : main_color;
        draw_utils::draw_outlined_quad(draw_manager, GfMatrix4f(1), color, color, pin_quad, 0.f, 0, m_pin_to_handle_id[pin]);
    };

    auto draw_vert_pins = [&](bool left) {
        const float x = left ? m_start[0] : m_end[0];
        const float left_edge = x - pin_vert_width;
        const float right_edge = x + pin_vert_width;

        // top
        RegionPin pin = left ? RegionPin::TopLeft : RegionPin::TopRight;
        std::vector<GfVec3f> pin_quad = { GfVec3f(left_edge, m_start[1] - pin_horiz_width, 0), GfVec3f(right_edge, m_start[1] - pin_horiz_width, 0),
                                          GfVec3f(right_edge, m_start[1] + pin_vert_length, 0), GfVec3f(left_edge, m_start[1] + pin_vert_length, 0) };
        auto color = m_selected_pin == pin ? select_color : main_color;
        draw_utils::draw_outlined_quad(draw_manager, GfMatrix4f(1), color, color, pin_quad, 0.f, 0, m_pin_to_handle_id[pin]);

        // middle
        pin = left ? RegionPin::Left : RegionPin::Right;
        const float middle_y = (m_end[1] + m_start[1]) / 2;
        pin_quad = { GfVec3f(left_edge, middle_y - pin_vert_length, 0), GfVec3f(right_edge, middle_y - pin_vert_length, 0),
                     GfVec3f(right_edge, middle_y + pin_vert_length, 0), GfVec3f(left_edge, middle_y + pin_vert_length, 0) };
        color = m_selected_pin == pin ? select_color : main_color;
        draw_utils::draw_outlined_quad(draw_manager, GfMatrix4f(1), color, color, pin_quad, 0.f, 0, m_pin_to_handle_id[pin]);

        // bottom
        pin = left ? RegionPin::BottomLeft : RegionPin::BottomRight;
        pin_quad = { GfVec3f(left_edge, m_end[1] - pin_vert_length, 0), GfVec3f(right_edge, m_end[1] - pin_vert_length, 0),
                     GfVec3f(right_edge, m_end[1] + pin_horiz_width, 0), GfVec3f(left_edge, m_end[1] + pin_horiz_width, 0) };
        color = m_selected_pin == pin ? select_color : main_color;
        draw_utils::draw_outlined_quad(draw_manager, GfMatrix4f(1), color, color, pin_quad, 0.f, 0, m_pin_to_handle_id[pin]);
    };

    // draw border
    draw_manager->begin_drawable(m_pin_to_handle_id[RegionPin::Border]);
    draw_manager->set_color(m_selected_pin == RegionPin::Border ? select_color : main_color);
    draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLinesStrip);
    draw_manager->set_depth_priority(1);
    draw_manager->rect2d(m_start, m_end);
    if (!m_mouse_inside_region)
        draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::STIPPLED);
    draw_manager->end_drawable();

    if (m_mouse_inside_region)
    {
        draw_horiz_pins(true);
        draw_horiz_pins(false);
        draw_vert_pins(true);
        draw_vert_pins(false);
    }
}

TfToken ViewportRenderRegionToolContext::get_name() const
{
    return RenderRegionToolTokens->name;
}

QCursor* ViewportRenderRegionToolContext::get_cursor()
{
    return m_cursor;
}

void ViewportRenderRegionToolContext::init_handle_ids(ViewportUiDrawManager* draw_manager)
{
    if (!draw_manager)
        return;

    for (int pin = 1; pin < static_cast<int>(RegionPin::Count); ++pin)
    {
        auto insert_result = m_pin_to_handle_id.emplace(static_cast<RegionPin>(pin), draw_manager->create_selection_id());
        m_handle_id_to_pin[insert_result.first->second] = static_cast<RegionPin>(pin);
    }
}

void ViewportRenderRegionToolContext::calc_corners(const ViewportViewPtr& viewport_view)
{
    const auto viewport_dim = viewport_view->get_viewport_dimensions();
    m_start = { 2 * float(m_region.left()) / viewport_dim.width - 1, 1 - 2 * float(m_region.top()) / viewport_dim.height };
    m_end = { 2 * float(m_region.right()) / viewport_dim.width - 1, 1 - 2 * float(m_region.bottom()) / viewport_dim.height };
}

bool ViewportRenderRegionToolContext::region_valid()
{
    return m_start[0] < m_end[0] && m_start[1] > m_end[1];
}

ViewportRenderRegionSession::ViewportRenderRegionSession()
    : m_start(0, 0)
    , m_end(0, 0)
{
}

ViewportRenderRegionSession& ViewportRenderRegionSession::instance()
{
    static ViewportRenderRegionSession session;
    return session;
}

GfVec2f& ViewportRenderRegionSession::get_start()
{
    return m_start;
}

GfVec2f& ViewportRenderRegionSession::get_end()
{
    return m_end;
}

QRect ViewportRenderRegionSession::get_rect(const ViewportViewPtr& viewport_view)
{
    const auto viewport_dim = viewport_view->get_viewport_dimensions();
    int left = (m_start[0] + 1) * viewport_dim.width / 2;
    int top = (1 - m_start[1]) * viewport_dim.height / 2;
    int right = (m_end[0] + 1) * viewport_dim.width / 2;
    int bottom = (1 - m_end[1]) * viewport_dim.height / 2;
    return QRect(QPoint(left, top), QPoint(right, bottom));
}

OPENDCC_NAMESPACE_CLOSE
