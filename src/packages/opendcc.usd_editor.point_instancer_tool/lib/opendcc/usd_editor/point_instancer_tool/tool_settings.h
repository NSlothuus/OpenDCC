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

OPENDCC_NAMESPACE_OPEN
class PointInstancerToolContext;

class PointInstancerToolSettingsWidget : public QWidget
{
    Q_OBJECT;

public:
    explicit PointInstancerToolSettingsWidget(PointInstancerToolContext* tool_context);
    ~PointInstancerToolSettingsWidget();

private:
    void update_type_combo_box(PointInstancerToolContext* tool_context, bool reset = false);
    Settings::SettingChangedHandle m_radius_changed;
    QComboBox* m_type_combo_box = nullptr;
};
OPENDCC_NAMESPACE_CLOSE
