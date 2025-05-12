/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/common_tools/viewport_select_tool_context.h"
#include <QCursor>

namespace PXR_NS
{
    TF_DECLARE_PUBLIC_TOKENS(RenderRegionToolTokens, ((name, "render_region_tool")));
};

OPENDCC_NAMESPACE_OPEN

class ViewportRenderRegionSession
{
public:
    static ViewportRenderRegionSession& instance();
    PXR_NS::GfVec2f& get_start();
    PXR_NS::GfVec2f& get_end();
    QRect get_rect(const ViewportViewPtr& viewport_view);

    ViewportRenderRegionSession(const ViewportRenderRegionSession&) = delete;
    ViewportRenderRegionSession(ViewportRenderRegionSession&&) = delete;
    ViewportRenderRegionSession& operator=(const ViewportRenderRegionSession&) = delete;
    ViewportRenderRegionSession& operator=(ViewportRenderRegionSession&&) = delete;

private:
    ViewportRenderRegionSession();

    PXR_NS::GfVec2f m_start;
    PXR_NS::GfVec2f m_end;
};

class ViewportRenderRegionToolContext : public ViewportSelectToolContext
{
    enum class RegionPin
    {
        None,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
        Border,
        Count
    };

public:
    ViewportRenderRegionToolContext();
    ~ViewportRenderRegionToolContext();
    ViewportRenderRegionToolContext(const ViewportRenderRegionToolContext&) = delete;
    ViewportRenderRegionToolContext(ViewportRenderRegionToolContext&&) = delete;
    ViewportRenderRegionToolContext& operator=(const ViewportRenderRegionToolContext&) = delete;
    ViewportRenderRegionToolContext& operator=(ViewportRenderRegionToolContext&&) = delete;

    virtual bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                               ViewportUiDrawManager* draw_manager) override;
    virtual bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view,
                                  ViewportUiDrawManager* draw_manager) override;
    virtual void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    virtual PXR_NS::TfToken get_name() const override;
    virtual QCursor* get_cursor() override;

private:
    QRect m_region;
    QRect m_selection;
    QPoint m_mouse_prev_pos;
    QCursor* m_cursor;

    PXR_NS::GfVec2f& m_start;
    PXR_NS::GfVec2f& m_end;

    std::unordered_map<RegionPin, uint32_t> m_pin_to_handle_id;
    std::unordered_map<uint32_t, RegionPin> m_handle_id_to_pin;
    RegionPin m_selected_pin = RegionPin::None;
    bool m_move_mode = false;
    bool m_mouse_inside_region = false;

    void init_handle_ids(ViewportUiDrawManager* draw_manager);
    void calc_corners(const ViewportViewPtr& viewport_view);
    bool region_valid();
};

OPENDCC_NAMESPACE_CLOSE
