/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"

#include <QWidget>
#include "opendcc/app/core/settings.h"

class QComboBox;
class QHBoxLayout;
class QLineEdit;

OPENDCC_NAMESPACE_OPEN

class PaintPrimvarToolContext;
class LadderNumberWidget;

class PaintPrimvarToolSettingsWidget : public QWidget
{
    Q_OBJECT;

public:
    explicit PaintPrimvarToolSettingsWidget(PaintPrimvarToolContext* tool_context);
    ~PaintPrimvarToolSettingsWidget();

private:
    Settings::SettingChangedHandle m_radius_changed;
    void update(PaintPrimvarToolContext* tool_context, bool reset = false);
    QComboBox* m_primvar_combo_box = nullptr;
    LadderNumberWidget* float_value_widget = nullptr;
    QWidget* vec3f_value_widget = nullptr;
    std::vector<QLineEdit*> m_edits;
};
OPENDCC_NAMESPACE_CLOSE
