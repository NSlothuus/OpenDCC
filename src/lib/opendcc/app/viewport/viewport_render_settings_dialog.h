/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QDialog>
#include <pxr/base/tf/token.h>
#include <pxr/imaging/hd/renderDelegate.h>

class QFormLayout;

OPENDCC_NAMESPACE_OPEN
class ViewportWidget;

class ViewportRenderSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    ViewportRenderSettingsDialog(ViewportWidget* viewport_widget, QWidget* parent = nullptr);

public Q_SLOTS:
    void on_render_plugin_changed(const PXR_NS::TfToken& render_plugin);
private Q_SLOTS:
    void restore_defaults();

private:
    void update_settings();
    QWidget* create_attribute_widget(const PXR_NS::TfToken& key, const PXR_NS::VtValue& value);
    void update_setting(QWidget* widget, const PXR_NS::TfToken& key, const PXR_NS::VtValue& value);

    PXR_NS::TfToken m_current_render_plugin;
    PXR_NS::HdRenderSettingDescriptorList m_render_settings;
    std::vector<QWidget*> m_settings_widgets;

    ViewportWidget* m_viewport_widget = nullptr;
    QFormLayout* m_settings_layout = nullptr;
};
OPENDCC_NAMESPACE_CLOSE