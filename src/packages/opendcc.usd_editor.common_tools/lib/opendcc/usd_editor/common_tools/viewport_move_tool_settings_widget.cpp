// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include "opendcc/usd_editor/common_tools/viewport_move_tool_settings_widget.h"
#include "opendcc/ui/common_widgets/ladder_widget.h"
#include "opendcc/ui/common_widgets/number_value_widget.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/viewport/tool_settings_view.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include <QButtonGroup>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <array>
#include "opendcc/ui/common_widgets/rollout_widget.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

REGISTER_TOOL_SETTINGS_VIEW(MoveToolTokens->name, TfToken("USD"), ViewportMoveToolContext, ViewportMoveToolSettingsWidget);

ViewportMoveToolSettingsWidget::ViewportMoveToolSettingsWidget(ViewportMoveToolContext* tool_context)
    : ViewportSelectToolSettingsWidget(tool_context)
{
    using AxisOrientation = ViewportMoveToolContext::AxisOrientation;
    using SnapMode = ViewportMoveToolContext::SnapMode;
    if (!TF_VERIFY(tool_context, "Invalid tool context."))
    {
        return;
    }

    static const QStringList axis_orientation_names = { i18n("tool_settings.viewport.move_tool.axis_orientation_names", "Object"),
                                                        i18n("tool_settings.viewport.move_tool.axis_orientation_names", "World") };
    static const QStringList snap_mode_names = { i18n("tool_settings.viewport.move_tool.snap_mode_names", "Off"),
                                                 i18n("tool_settings.viewport.move_tool.snap_mode_names", "Relative"),
                                                 i18n("tool_settings.viewport.move_tool.snap_mode_names", "Absolute"),
                                                 i18n("tool_settings.viewport.move_tool.snap_mode_names", "Grid"),
                                                 i18n("tool_settings.viewport.move_tool.snap_mode_names", "Vertex"),
                                                 i18n("tool_settings.viewport.move_tool.snap_mode_names", "Edge"),
                                                 i18n("tool_settings.viewport.move_tool.snap_mode_names", "Edge Center"),
                                                 i18n("tool_settings.viewport.move_tool.snap_mode_names", "Face Center"),
                                                 i18n("tool_settings.viewport.move_tool.snap_mode_names", "Object Surface") };
    static const QStringList snap_mode_icons = { ":/icons/small_snap_off",           ":/icons/small_relative",
                                                 ":/icons/small_absolute",           ":/icons/small_snap_grid",
                                                 ":/icons/small_snap_vertex",        ":/icons/small_snap_edge",
                                                 ":/icons/small_snap_edge_center",   ":/icons/small_snap_face_center",
                                                 ":/icons/small_snap_object_surface" };

    auto axis_orientation_cb = new QComboBox;
    axis_orientation_cb->addItems(axis_orientation_names);
    axis_orientation_cb->setCurrentIndex(static_cast<int>(tool_context->get_axis_orientation()));
    axis_orientation_cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    axis_orientation_cb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    connect(axis_orientation_cb, qOverload<int>(&QComboBox::activated), this, [tool_context](int axis_orientation_index) {
        tool_context->set_axis_orientation(static_cast<AxisOrientation>(axis_orientation_index));

        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });

    auto pivot_layout = new QHBoxLayout;

    auto edit_pivot_btn = new QPushButton(i18n("tool_settings.viewport.move_tool", "Edit Pivot"));
    edit_pivot_btn->setCheckable(true);
    edit_pivot_btn->setChecked(false);
    connect(edit_pivot_btn, &QPushButton::clicked, this, [tool_context](bool checked) {
        tool_context->set_edit_pivot(checked);
        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });
    connect(tool_context, &ViewportMoveToolContext::edit_pivot_mode_enabled, this, [edit_pivot_btn](bool enabled) {
        if (edit_pivot_btn->isChecked() == enabled)
            return;
        edit_pivot_btn->setChecked(enabled);
    });

    auto reset_pivot_btn = new QPushButton(i18n("tool_settings.viewport.move_tool", "Reset"));
    connect(reset_pivot_btn, &QPushButton::clicked, this, [tool_context] {
        tool_context->reset_pivot();
        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });

    pivot_layout->addWidget(edit_pivot_btn, 2);
    pivot_layout->addWidget(reset_pivot_btn, 1);
    pivot_layout->addStretch(2);
    pivot_layout->setContentsMargins(0, 0, 0, 0);

    auto snap_layout = new QHBoxLayout;

    auto step_widget = new LadderNumberWidget(nullptr, false);
    step_widget->set_clamp(0.0, 100000);
    step_widget->enable_clamp(true);
    step_widget->setText(QString::number(tool_context->get_step()));
    step_widget->setEnabled(tool_context->get_snap_mode() == SnapMode::RELATIVE_MODE || tool_context->get_snap_mode() == SnapMode::ABSOLUTE_MODE);
    connect(step_widget, &LadderNumberWidget::editingFinished, this,
            [tool_context, step_widget] { tool_context->set_step(step_widget->text().toDouble()); });

    auto snap_mode_cb = new QComboBox;
    for (int i = 0; i < snap_mode_names.length(); i++)
        snap_mode_cb->addItem(QIcon(snap_mode_icons[i]), snap_mode_names[i]);
    snap_mode_cb->setCurrentIndex(static_cast<int>(tool_context->get_snap_mode()));
    snap_mode_cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    snap_mode_cb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    connect(snap_mode_cb, qOverload<int>(&QComboBox::activated), this,
            [tool_context](int step_mode_index) { Application::instance().get_settings()->set("viewport.move_tool.snap_mode", step_mode_index); });

    snap_layout->addWidget(snap_mode_cb, 2);
    snap_layout->addWidget(step_widget, 1);
    snap_layout->addStretch(3);
    snap_layout->setContentsMargins(0, 0, 0, 0);

    auto content_layout = new QGridLayout;
    content_layout->setColumnStretch(0, 2);
    content_layout->setColumnStretch(1, 5);

    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.move_tool", "Axis Orientation:")), 0, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(axis_orientation_cb, 0, 1, Qt::AlignmentFlag::AlignVCenter);

    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.move_tool", "Pivot:")), 1, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignVCenter);
    content_layout->addLayout(pivot_layout, 1, 1, Qt::AlignmentFlag::AlignVCenter);

    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.move_tool", "Snap:")), 2, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addLayout(snap_layout, 2, 1, Qt::AlignmentFlag::AlignVCenter);

    auto rollout = new RolloutWidget(i18n("tool_settings.viewport.move_tool", "Move Settings"));
    auto settings = Application::instance().get_settings();
    bool expanded = settings->get("viewport.move_tool.ui.move_settings", true);
    rollout->set_expanded(expanded);
    connect(rollout, &RolloutWidget::clicked, [](bool expanded) {
        auto settings = Application::instance().get_settings();
        settings->set("viewport.move_tool.ui.move_settings", !expanded);
    });

    rollout->set_layout(content_layout);

    qobject_cast<QVBoxLayout*>(this->layout())->insertWidget(0, rollout);

    m_settings_changed_cid["viewport.move_tool.axis_orientation"] = settings->register_setting_changed(
        "viewport.move_tool.axis_orientation", [axis_orientation_cb](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            axis_orientation_cb->setCurrentIndex(val.get(0));
        });
    m_settings_changed_cid["viewport.move_tool.step"] = settings->register_setting_changed(
        "viewport.move_tool.step", [step_widget](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            step_widget->setText(QString::number(val.get<double>()));
        });
    m_settings_changed_cid["viewport.move_tool.snap_mode"] = settings->register_setting_changed(
        "viewport.move_tool.snap_mode",
        [snap_mode_cb, step_widget, tool_context](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            snap_mode_cb->setCurrentIndex(val.get<int>());
            const auto mode = static_cast<SnapMode>(snap_mode_cb->currentIndex());
            if (mode == SnapMode::RELATIVE_MODE || mode == SnapMode::ABSOLUTE_MODE)
                step_widget->setEnabled(true);
            else
                step_widget->setEnabled(false);
        });
}

ViewportMoveToolSettingsWidget::~ViewportMoveToolSettingsWidget()
{
    for (const auto& entry : m_settings_changed_cid)
        Application::instance().get_settings()->unregister_setting_changed(entry.first, entry.second);
}

OPENDCC_NAMESPACE_CLOSE
