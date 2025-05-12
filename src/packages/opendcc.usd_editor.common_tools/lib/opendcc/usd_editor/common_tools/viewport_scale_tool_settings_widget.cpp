// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include "opendcc/usd_editor/common_tools/viewport_scale_tool_settings_widget.h"
#include "opendcc/app/viewport/tool_settings_view.h"
#include "opendcc/ui/common_widgets/ladder_widget.h"
#include "QComboBox"
#include "QGroupBox"
#include "QPushButton"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include "opendcc/ui/common_widgets/rollout_widget.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

REGISTER_TOOL_SETTINGS_VIEW(ScaleToolTokens->name, TfToken("USD"), ViewportScaleToolContext, ViewportScaleToolSettingsWidget);

ViewportScaleToolSettingsWidget::ViewportScaleToolSettingsWidget(ViewportScaleToolContext* tool_context)
    : ViewportSelectToolSettingsWidget(tool_context)
{
    using StepMode = ViewportScaleToolContext::StepMode;
    static const QStringList step_mode_names = { i18n("tool_settings.viewport.scale_tool", "Off"),
                                                 i18n("tool_settings.viewport.scale_tool", "Relative"),
                                                 i18n("tool_settings.viewport.scale_tool", "Absolute") };

    auto pivot_layout = new QHBoxLayout;
    auto edit_pivot_btn = new QPushButton(i18n("tool_settings.viewport.scale_tool", "Edit Pivot"));
    edit_pivot_btn->setCheckable(true);
    edit_pivot_btn->setChecked(false);
    connect(edit_pivot_btn, &QPushButton::clicked, this, [tool_context](bool checked) {
        tool_context->set_edit_pivot(checked);
        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });
    connect(tool_context, &ViewportScaleToolContext::edit_pivot_mode_enabled, this, [edit_pivot_btn](bool enabled) {
        if (edit_pivot_btn->isChecked() == enabled)
            return;
        edit_pivot_btn->setChecked(enabled);
    });

    auto reset_pivot_btn = new QPushButton(i18n("tool_settings.viewport.scale_tool", "Reset"));
    connect(reset_pivot_btn, &QPushButton::clicked, this, [tool_context] {
        tool_context->reset_pivot();
        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });

    pivot_layout->addWidget(edit_pivot_btn, 2);
    pivot_layout->addWidget(reset_pivot_btn, 1);
    pivot_layout->addStretch(2);
    pivot_layout->setContentsMargins(0, 0, 0, 0);

    auto step_snap_layout = new QHBoxLayout;

    auto step_widget = new LadderNumberWidget(nullptr, false);
    step_widget->set_clamp(0.0, 100000);
    step_widget->enable_clamp(true);
    step_widget->setText(QString::number(tool_context->get_step()));
    step_widget->setEnabled(tool_context->get_step_mode() != StepMode::OFF);
    connect(step_widget, &LadderNumberWidget::editingFinished, this,
            [tool_context, step_widget] { tool_context->set_step(step_widget->text().toDouble()); });

    auto step_mode_cb = new QComboBox;
    step_mode_cb->addItems(step_mode_names);
    step_mode_cb->setCurrentIndex(static_cast<int>(tool_context->get_step_mode()));
    step_mode_cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    step_mode_cb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    connect(step_mode_cb, qOverload<int>(&QComboBox::activated), this, [tool_context](int step_mode_index) {
        const auto mode = static_cast<StepMode>(step_mode_index);
        tool_context->set_step_mode(mode);
    });

    step_snap_layout->addWidget(step_mode_cb, 2);
    step_snap_layout->addWidget(step_widget, 1);
    step_snap_layout->addStretch(3);
    step_snap_layout->setContentsMargins(0, 0, 0, 0);

    auto content_layout = new QGridLayout;
    content_layout->setColumnStretch(0, 2);
    content_layout->setColumnStretch(1, 5);

    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.scale_tool", "Pivot:")), 0, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignVCenter);
    content_layout->addLayout(pivot_layout, 0, 1, Qt::AlignmentFlag::AlignVCenter);

    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.scale_tool", "Step Snap:")), 1, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addLayout(step_snap_layout, 1, 1, Qt::AlignmentFlag::AlignVCenter);

    auto rollout = new RolloutWidget(i18n("tool_settings.viewport.scale_tool", "Scale Settings"));
    auto settings = Application::instance().get_settings();
    bool expanded = settings->get("viewport.scale_tool.ui.scale_settings", true);
    rollout->set_expanded(expanded);
    connect(rollout, &RolloutWidget::clicked, [](bool expanded) {
        auto settings = Application::instance().get_settings();
        settings->set("viewport.scale_tool.ui.scale_settings", !expanded);
    });

    rollout->set_layout(content_layout);

    qobject_cast<QVBoxLayout*>(this->layout())->insertWidget(0, rollout);

    m_settings_changed_cid["viewport.scale_tool.step"] = settings->register_setting_changed(
        "viewport.scale_tool.step", [step_widget](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            double step = 0;
            if (!val.try_get<double>(&step))
                return;
            step_widget->setText(QString::number(step));
        });
    m_settings_changed_cid["viewport.scale_tool.step_mode"] = settings->register_setting_changed(
        "viewport.scale_tool.step_mode", [step_mode_cb, step_widget](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            int ind = 0;
            if (!val.try_get<int>(&ind))
                return;
            step_mode_cb->setCurrentIndex(ind);
            step_widget->setEnabled(step_mode_cb->currentIndex() != 0);
        });
}

ViewportScaleToolSettingsWidget::~ViewportScaleToolSettingsWidget()
{
    for (const auto& entry : m_settings_changed_cid)
        Application::instance().get_settings()->unregister_setting_changed(entry.first, entry.second);
}

OPENDCC_NAMESPACE_CLOSE
