/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/ui/application_ui.h"
#include <QWidget>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API ToolSettingsView : public QWidget
{
    Q_OBJECT;

public:
    explicit ToolSettingsView(QWidget* parent = nullptr);
    ~ToolSettingsView();

private:
    void on_viewport_tool_changed();

    Application::CallbackHandle m_tool_changed_handle;
};

class OPENDCC_API ToolSettingsViewRegistry
{
public:
    static ToolSettingsViewRegistry& instance();

    static bool register_tool_settings_view(const PXR_NS::TfToken& name, const PXR_NS::TfToken& context, std::function<QWidget*()> factory_fn);
    static bool unregister_tool_settings_view(const PXR_NS::TfToken& name, const PXR_NS::TfToken& context);

    static QWidget* create_tool_settings_widget(const PXR_NS::TfToken& name, const PXR_NS::TfToken& context);

private:
    ToolSettingsViewRegistry() = default;

    std::unordered_map<PXR_NS::TfToken, std::unordered_map<PXR_NS::TfToken, std::function<QWidget*()>, PXR_NS::TfToken::HashFunctor>,
                       PXR_NS::TfToken::HashFunctor>
        m_registry;
};

#define REGISTER_TOOL_SETTINGS_VIEW(name, context, context_type, widget_type)                                                            \
    static bool context_type##_tool_settings_view_registered = ToolSettingsViewRegistry::register_tool_settings_view(name, context, [] { \
        auto derived_context = dynamic_cast<context_type*>(ApplicationUI::instance().get_current_viewport_tool());                       \
        return derived_context ? new widget_type(derived_context) : nullptr;                                                             \
    });

OPENDCC_NAMESPACE_CLOSE
