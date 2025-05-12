/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QWidget>
#include "opendcc/usd_editor/common_tools/viewport_move_tool_context.h"
#include "opendcc/usd_editor/common_tools/viewport_select_tool_settings_widget.h"

OPENDCC_NAMESPACE_OPEN

class ViewportMoveToolSettingsWidget : public ViewportSelectToolSettingsWidget
{
    Q_OBJECT;

public:
    explicit ViewportMoveToolSettingsWidget(ViewportMoveToolContext* tool_context);
    ~ViewportMoveToolSettingsWidget();

private:
    std::unordered_map<std::string, Settings::SettingChangedHandle> m_settings_changed_cid;
};

OPENDCC_NAMESPACE_CLOSE
