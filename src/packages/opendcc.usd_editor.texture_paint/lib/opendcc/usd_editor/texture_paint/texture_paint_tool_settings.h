/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/settings.h"
#include <QWidget>

class QLineEdit;

OPENDCC_NAMESPACE_OPEN

class TexturePaintToolContext;

class TexturePaintToolSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    TexturePaintToolSettingsWidget(TexturePaintToolContext* tool_context);
    ~TexturePaintToolSettingsWidget() override;

private:
    QLineEdit* m_texture_file_widget = nullptr;
    Settings::SettingChangedHandle m_radius_changed;
    Settings::SettingChangedHandle m_path_changed;
    std::vector<QLineEdit*> m_edits;
};

OPENDCC_NAMESPACE_CLOSE
