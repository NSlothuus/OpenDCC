// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "snap_window.h"
#include <QBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QSpacerItem>
#include "opendcc/anim_engine/core/session.h"
#include "opendcc/app/ui/application_ui.h"

OPENDCC_NAMESPACE_OPEN

SnapWindow::SnapWindow()
{
    QHBoxLayout* radio_buttons_layout = new QHBoxLayout();
    time_radio_button.setText(i18n("graph_editor", "Time"));
    value_radio_button.setText(i18n("graph_editor", "Value"));
    both_radio_button.setText(i18n("graph_editor", "Both"));
    radio_buttons_layout->addWidget(&time_radio_button);
    radio_buttons_layout->addWidget(&value_radio_button);
    radio_buttons_layout->addWidget(&both_radio_button);
    time_radio_button.setChecked(true);

    QDoubleSpinBox* spinbox_snap_time_interval = new QDoubleSpinBox();
    spinbox_snap_time_interval->setValue(1);
    QDoubleSpinBox* spinbox_snap_value_interval = new QDoubleSpinBox();
    spinbox_snap_value_interval->setValue(1);

    snap_keys_button = new QPushButton(i18n("graph_editor", "Snap Keys"));
    apply_button = new QPushButton(i18n("graph_editor", "Apply"));
    close_button = new QPushButton(i18n("graph_editor", "Close"));

    QGridLayout* grid_layout = new QGridLayout();
    grid_layout->addWidget(new QLabel(i18n("graph_editor", "Snap") + ":"), 0, 0);
    grid_layout->addLayout(radio_buttons_layout, 0, 1);
    grid_layout->addWidget(new QLabel(i18n("graph_editor", "Snap times to multiple of") + ":"), 1, 0);
    grid_layout->addWidget(spinbox_snap_time_interval, 1, 1);
    grid_layout->addWidget(new QLabel(i18n("graph_editor", "Snap value to multiple of") + ":"), 2, 0);
    grid_layout->addWidget(spinbox_snap_value_interval, 2, 1);

    QHBoxLayout* h_layout = new QHBoxLayout();
    h_layout->addWidget(snap_keys_button);
    h_layout->addWidget(apply_button);
    h_layout->addWidget(close_button);

    QVBoxLayout* v_layout = new QVBoxLayout();
    v_layout->addLayout(grid_layout);

    v_layout->addItem(new QSpacerItem(10, 1000, QSizePolicy::Maximum, QSizePolicy::Maximum));
    v_layout->addLayout(h_layout);

    setLayout(v_layout);

    resize(300, 300);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    connect(spinbox_snap_time_interval, SIGNAL(valueChanged(double)), this, SIGNAL(snap_time_change_signal(double)));
    connect(spinbox_snap_value_interval, SIGNAL(valueChanged(double)), this, SIGNAL(snap_value_change_signal(double)));
    connect(snap_keys_button, SIGNAL(clicked()), this, SLOT(snap_selection()));
    connect(snap_keys_button, SIGNAL(clicked()), this, SLOT(hide()));
    connect(apply_button, SIGNAL(clicked()), this, SLOT(snap_selection()));
    connect(close_button, SIGNAL(clicked()), this, SLOT(hide()));
}

SnapWindow::~SnapWindow() {}

void SnapWindow::snap_selection()
{
    if (time_radio_button.isChecked())
    {
        emit snap_selection_signal(true, false);
    }
    else if (value_radio_button.isChecked())
    {
        emit snap_selection_signal(false, true);
    }
    else
    {
        emit snap_selection_signal(true, true);
    }
}

bool SnapWindow::is_snap_time()
{
    return (time_radio_button.isChecked() || both_radio_button.isChecked());
}

bool SnapWindow::is_snap_value()
{
    return (value_radio_button.isChecked() || both_radio_button.isChecked());
}
OPENDCC_NAMESPACE_CLOSE
