/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/bezier_tool/bezier_tool_context.h"

#include <QWidget>

class QVBoxLayout;

OPENDCC_NAMESPACE_OPEN

class BezierToolSettingsWidget : public QWidget
{
    Q_OBJECT;

public:
    explicit BezierToolSettingsWidget(BezierToolContext* context);
    ~BezierToolSettingsWidget() override;

private:
    void createManipSettingsRollout();
    void createTangentSettingsRollout();
    void createHotkeysRollout();

    BezierToolContext* m_context = nullptr;
    QVBoxLayout* m_layout = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
