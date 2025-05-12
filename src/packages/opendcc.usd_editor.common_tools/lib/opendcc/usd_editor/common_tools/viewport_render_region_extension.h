/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/iviewport_ui_extension.h"
#include "opendcc/usd_editor/common_tools/viewport_render_region_tool_context.h"

OPENDCC_NAMESPACE_OPEN

class ViewportRenderRegionExtension : public IViewportUIExtension
{
public:
    ViewportRenderRegionExtension(ViewportWidget* viewport_widget);
    std::vector<IViewportDrawExtensionPtr> create_draw_extensions() override;

    class ViewportRenderRegionDrawExtension : public IViewportDrawExtension
    {
    public:
        ViewportRenderRegionDrawExtension(ViewportGLWidget* gl_widget, ViewportViewPtr viewport_view);
        virtual void draw(ViewportUiDrawManager* draw_manager, const PXR_NS::GfFrustum& frustum, int width, int height) override;
        void set_enabled(bool enable);

    private:
        bool m_enabled = false;
        ViewportGLWidget* m_gl_widget;
        ViewportViewPtr m_viewport_view;
    };

private:
    std::shared_ptr<ViewportRenderRegionDrawExtension> m_draw_extension;
};

OPENDCC_NAMESPACE_CLOSE
