// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include "opendcc/app/viewport/iviewport_ui_extension.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

ViewportUIExtensionRegistry& ViewportUIExtensionRegistry::instance()
{
    static ViewportUIExtensionRegistry instance;
    return instance;
}

void ViewportUIExtensionRegistry::register_ui_extension(const PXR_NS::TfToken& name,
                                                        std::function<IViewportUiExtensionPtr(ViewportWidget*)> factory_fn)
{
    if (!factory_fn)
    {
        OPENDCC_ERROR("Failed to register UI Extension '{}': factory function is empty.", name.GetString());
    }
    if (!m_registry.emplace(name, std::move(factory_fn)).second)
    {
        OPENDCC_WARN("'{}' UI Extension already registered.", name.GetString());
    }
}

void ViewportUIExtensionRegistry::unregister_ui_extension(const PXR_NS::TfToken& name)
{
    if (!m_registry.erase(name))
    {
        OPENDCC_WARN("Failed to remove UI Extension '{}': not registered.", name.GetString());
    }
}

std::vector<IViewportUiExtensionPtr> ViewportUIExtensionRegistry::create_extensions(ViewportWidget* viewport_widget)
{
    std::vector<IViewportUiExtensionPtr> result;
    std::vector<IViewportDrawExtensionPtr> all_draw_extensions;
    const auto& registry = instance().m_registry;

    for (const auto& [name, factory_fn] : registry)
    {
        if (auto ui_ext = factory_fn(viewport_widget))
        {
            result.push_back(ui_ext);
            auto draw_layers = ui_ext->create_draw_extensions();
            all_draw_extensions.insert(all_draw_extensions.end(), draw_layers.begin(), draw_layers.end());
        }
    }

    if (!all_draw_extensions.empty())
    {
        viewport_widget->get_gl_widget()->set_draw_extensions(all_draw_extensions);
    }

    return result;
}

OPENDCC_NAMESPACE_CLOSE
