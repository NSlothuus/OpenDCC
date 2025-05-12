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

OPENDCC_NAMESPACE_OPEN

class LadderNumberWidget;
class SculptToolContext;

class SculptToolSettingsWidget : public QWidget
{
    Q_OBJECT;

public:
    explicit SculptToolSettingsWidget(SculptToolContext* tool_context);
    ~SculptToolSettingsWidget();

private:
    Settings::SettingChangedHandle m_radius_changed;
    void update(SculptToolContext* tool_context, bool reset = false);
};
OPENDCC_NAMESPACE_CLOSE
