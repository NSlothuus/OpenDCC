// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/common_tools/viewport_render_region_extension.h"
#include "opendcc/app/viewport/viewport_ui_draw_manager.h"
#include "opendcc/app/viewport/iviewport_ui_extension.h"
#include "opendcc/app/ui/application_ui.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include <QAction>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

ViewportRenderRegionExtension::ViewportRenderRegionExtension(ViewportWidget* viewport_widget)
    : IViewportUIExtension(viewport_widget)
{
    m_draw_extension = std::make_shared<ViewportRenderRegionDrawExtension>(viewport_widget->get_gl_widget(), viewport_widget->get_viewport_view());
    auto render_region = new QAction(QIcon(":icons/small_regionSelectKeySmall.png"), i18n("viewport.actions", "Render Region"), viewport_widget);
    render_region->setCheckable(true);
    render_region->setChecked(false);
    viewport_widget->connect(render_region, &QAction::triggered, viewport_widget, [this](bool checked) {
        auto tool_ctx_name = checked ? RenderRegionToolTokens->name : SelectToolTokens->name;
        m_draw_extension->set_enabled(checked);

        auto gl_widget = get_viewport_widget()->get_gl_widget();
        if (!checked)
            gl_widget->set_crop_region(GfRect2i()); // set an invalid rect - thus we disable framing
        gl_widget->update();

        const auto current_context = ApplicationUI::instance().get_current_viewport_tool();
        if (current_context && current_context->get_name() == tool_ctx_name)
            return;

        const auto tool_context =
            ViewportToolContextRegistry::create_tool_context(Application::instance().get_active_view_scene_context(), tool_ctx_name);
        ApplicationUI::instance().set_current_viewport_tool(tool_context);
    });
    viewport_widget->toolbar_add_action(render_region);
}

std::vector<IViewportDrawExtensionPtr> ViewportRenderRegionExtension::create_draw_extensions()
{
    std::vector<IViewportDrawExtensionPtr> result;
    result.push_back(m_draw_extension);
    return result;
}

ViewportRenderRegionExtension::ViewportRenderRegionDrawExtension::ViewportRenderRegionDrawExtension(ViewportGLWidget* gl_widget,
                                                                                                    ViewportViewPtr viewport_view)
    : IViewportDrawExtension()
    , m_gl_widget(gl_widget)
    , m_viewport_view(viewport_view)
{
}

void ViewportRenderRegionExtension::ViewportRenderRegionDrawExtension::draw(ViewportUiDrawManager* draw_manager, const GfFrustum& frustum, int width,
                                                                            int height)
{
    if (!m_enabled)
        return;

    // framing
    auto qrect = ViewportRenderRegionSession::instance().get_rect(m_viewport_view).adjusted(1, 1, -1, -1);
    GfRect2i render_rect(GfVec2i(qrect.left(), qrect.top()), qrect.width(), qrect.height());
    if (render_rect != m_gl_widget->get_crop_region())
    {
        m_gl_widget->set_crop_region(render_rect);
        m_gl_widget->update();
    }

    // drawing a border to remain when the ToolContext is disabled
    draw_manager->begin_drawable();
    draw_manager->set_color(GfVec4f(0, 0, 0, 1));
    draw_manager->set_prim_type(ViewportUiDrawManager::PrimitiveTypeLinesStrip);
    draw_manager->rect2d(ViewportRenderRegionSession::instance().get_start(), ViewportRenderRegionSession::instance().get_end());
    draw_manager->set_paint_style(ViewportUiDrawManager::PaintStyle::STIPPLED);
    draw_manager->set_depth_priority(1);
    draw_manager->end_drawable();
}

void ViewportRenderRegionExtension::ViewportRenderRegionDrawExtension::set_enabled(bool enable)
{
    m_enabled = enable;
}

OPENDCC_NAMESPACE_CLOSE
