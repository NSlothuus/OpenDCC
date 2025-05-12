/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QWidget>
#include "opendcc/usd_editor/common_tools/viewport_select_tool_context.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/settings.h"

class QVBoxLayout;

OPENDCC_NAMESPACE_OPEN

class ViewportSelectToolSettingsWidget : public QWidget
{
    Q_OBJECT;

public:
    explicit ViewportSelectToolSettingsWidget(ViewportSelectToolContext* tool_context);
    ~ViewportSelectToolSettingsWidget() override;

private:
    void init_common_selection_options();
    void init_soft_selection();

    Application::CallbackHandle m_selection_mask_changed_cid;
    QVBoxLayout* m_layout = nullptr;
    ViewportSelectToolContext* m_tool_context = nullptr;
    Application::CallbackHandle m_selection_changed_cid;
    std::unordered_map<std::string, Settings::SettingChangedHandle> m_settings_changed_cid;
};

OPENDCC_NAMESPACE_CLOSE
