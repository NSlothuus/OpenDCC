// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_context_menu_registry.h"
#include "opendcc/app/viewport/iviewport_tool_context.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

ViewportContextMenuRegistry& ViewportContextMenuRegistry::instance()
{
    static ViewportContextMenuRegistry instance;
    return instance;
}

bool ViewportContextMenuRegistry::register_menu(const TfToken& context_type, CreateContextMenuFn creator)
{
    auto emplace_result = m_registry.emplace(context_type, creator);
    if (!emplace_result.second)
    {
        TF_WARN("Failed to register viewport context menu for context '%s': context menu is already registered.", context_type.GetText());
        return false;
    }

    return true;
}

bool ViewportContextMenuRegistry::unregister_menu(const TfToken& context_type)
{
    auto erase_result = m_registry.erase(context_type);
    if (erase_result == 0)
    {
        TF_WARN("Failed to unregister viewport context menu for context '%s': context menu is not found.", context_type.GetText());
        return false;
    }

    return true;
}

QMenu* ViewportContextMenuRegistry::create_menu(const TfToken& context_type, QContextMenuEvent* context_menu_event, ViewportViewPtr viewport_view,
                                                QWidget* parent)
{
    auto default_iter = m_registry.find(context_type);
    if (default_iter != m_registry.end() && default_iter->second)
    {
        return default_iter->second(context_menu_event, viewport_view, parent);
    }
    TF_WARN("Failed to create viewport context menu for context '%s': context menu is not registered.", context_type.GetText());
    return nullptr;
}

OPENDCC_NAMESPACE_CLOSE