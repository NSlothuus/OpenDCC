/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"
#include "opendcc/app/viewport/iviewport_tool_context.h"

OPENDCC_NAMESPACE_OPEN

class HydraOpSelectToolContext : public IViewportToolContext
{
public:
    HydraOpSelectToolContext();
    ~HydraOpSelectToolContext() = default;

    bool on_mouse_press(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_move(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    bool on_mouse_release(const ViewportMouseEvent& mouse_event, const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;
    void draw(const ViewportViewPtr& viewport_view, ViewportUiDrawManager* draw_manager) override;

    PXR_NS::TfToken get_name() const override;

protected:
    bool is_locked() const;

    bool m_select_rect_mode;
    bool m_shift;
    bool m_ctrl;

private:
    int m_start_posx = 0;
    int m_start_posy = 0;

    int m_mousex = 0;
    int m_mousey = 0;
    std::unique_ptr<PXR_NS::GfVec3f> m_centroid;
    PXR_NS::GfVec3d m_start_world_pos;
};

OPENDCC_NAMESPACE_CLOSE
