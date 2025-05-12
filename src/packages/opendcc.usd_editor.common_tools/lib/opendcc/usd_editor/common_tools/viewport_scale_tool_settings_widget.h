/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QWidget>
#include "opendcc/usd_editor/common_tools/viewport_scale_tool_context.h"
#include "opendcc/usd_editor/common_tools/viewport_select_tool_settings_widget.h"

OPENDCC_NAMESPACE_OPEN

class ViewportScaleToolSettingsWidget : public ViewportSelectToolSettingsWidget
{
    Q_OBJECT;

public:
    explicit ViewportScaleToolSettingsWidget(ViewportScaleToolContext* tool_context);
    ~ViewportScaleToolSettingsWidget();

private:
    std::unordered_map<std::string, Settings::SettingChangedHandle> m_settings_changed_cid;
};

OPENDCC_NAMESPACE_CLOSE
