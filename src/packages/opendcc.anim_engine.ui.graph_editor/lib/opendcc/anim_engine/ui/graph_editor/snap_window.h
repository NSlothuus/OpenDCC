/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"

#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QWidgetAction>

OPENDCC_NAMESPACE_OPEN

class SnapWindow : public QWidget
{
    Q_OBJECT
public:
    SnapWindow();
    ~SnapWindow();

    bool is_snap_time();
    bool is_snap_value();

Q_SIGNALS:
    void snap_selection_signal(bool, bool);
    void snap_time_change_signal(double snap_time_interval);
    void snap_value_change_signal(double snap_value_interval);

private Q_SLOTS:
    void snap_selection();

private:
    QPushButton* snap_keys_button;
    QPushButton* apply_button;
    QPushButton* close_button;
    QRadioButton time_radio_button;
    QRadioButton value_radio_button;
    QRadioButton both_radio_button;
};
OPENDCC_NAMESPACE_CLOSE
