// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include "opendcc/usd_editor/common_tools/viewport_rotate_tool_settings_widget.h"
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGroupBox>
#include <array>
#include "opendcc/app/core/application.h"
#include "opendcc/app/viewport/tool_settings_view.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include "QComboBox"
#include "QFormLayout"
#include "QPushButton"
#include "opendcc/ui/common_widgets/ladder_widget.h"
#include "QCheckBox"
#include "opendcc/ui/common_widgets/rollout_widget.h"
#include "opendcc/app/ui/application_ui.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

REGISTER_TOOL_SETTINGS_VIEW(RotateToolTokens->name, TfToken("USD"), ViewportRotateToolContext, ViewportRotateToolSettingsWidget);

ViewportRotateToolSettingsWidget::ViewportRotateToolSettingsWidget(ViewportRotateToolContext* tool_context)
    : ViewportSelectToolSettingsWidget(tool_context)
{
    using AxisOrientation = ViewportRotateToolContext::Orientation;
    if (!TF_VERIFY(tool_context, "Invalid tool context."))
    {
        return;
    }

    static const QStringList axis_orientation_names = { "Object", "World", "Gimbal" };

    auto axis_orientation_cb = new QComboBox;
    axis_orientation_cb->addItems(axis_orientation_names);
    axis_orientation_cb->setCurrentIndex(static_cast<int>(tool_context->get_orientation()));
    axis_orientation_cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    axis_orientation_cb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    connect(axis_orientation_cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [tool_context](int axis_orientation_index) {
        tool_context->set_orientation(static_cast<ViewportRotateToolContext::Orientation>(axis_orientation_index));

        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });

    auto step_snap_layout = new QHBoxLayout;
    auto step_widget = new LadderNumberWidget(nullptr, false);
    step_widget->set_clamp(0.0, 100000.0);
    step_widget->enable_clamp(true);
    step_widget->setText(QString::number(tool_context->get_step()));
    step_widget->setEnabled(tool_context->is_step_mode_enabled());
    connect(step_widget, &LadderNumberWidget::editingFinished, this,
            [tool_context, step_widget] { tool_context->set_step(step_widget->text().toDouble()); });

    auto enable_step_mode_cb = new QCheckBox;
    enable_step_mode_cb->setChecked(tool_context->is_step_mode_enabled());
    connect(enable_step_mode_cb, &QCheckBox::clicked, this, [tool_context](bool enable) { tool_context->enable_step_mode(enable); });

    step_snap_layout->addWidget(enable_step_mode_cb);
    step_snap_layout->addWidget(step_widget, 1);
    step_snap_layout->addStretch(4);
    step_snap_layout->setContentsMargins(0, 0, 0, 0);

    auto pivot_layout = new QHBoxLayout;
    auto edit_pivot_btn = new QPushButton(i18n("tool_settings.viewport.rotate_tool", "Edit Pivot"));
    edit_pivot_btn->setCheckable(true);
    edit_pivot_btn->setChecked(false);
    connect(edit_pivot_btn, &QPushButton::clicked, this, [tool_context](bool checked) {
        tool_context->set_edit_pivot(checked);
        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });
    connect(tool_context, &ViewportRotateToolContext::edit_pivot_mode_enabled, this, [edit_pivot_btn](bool enabled) {
        if (edit_pivot_btn->isChecked() == enabled)
            return;
        edit_pivot_btn->setChecked(enabled);
    });

    auto reset_pivot_btn = new QPushButton(i18n("tool_settings.viewport.rotate_tool", "Reset"));
    connect(reset_pivot_btn, &QPushButton::clicked, this, [tool_context] {
        tool_context->reset_pivot();
        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });

    pivot_layout->addWidget(edit_pivot_btn, 2);
    pivot_layout->addWidget(reset_pivot_btn, 1);
    pivot_layout->addStretch(2);
    pivot_layout->setContentsMargins(0, 0, 0, 0);

    auto content_layout = new QGridLayout;
    content_layout->setColumnStretch(0, 2);
    content_layout->setColumnStretch(1, 5);

    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.rotate_tool", "Orientation:")), 0, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(axis_orientation_cb, 0, 1, Qt::AlignmentFlag::AlignVCenter);

    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.rotate_tool", "Pivot:")), 1, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignVCenter);
    content_layout->addLayout(pivot_layout, 1, 1, Qt::AlignmentFlag::AlignVCenter);

    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.rotate_tool", "Step Snap:")), 2, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addLayout(step_snap_layout, 2, 1, Qt::AlignmentFlag::AlignVCenter);

    auto rollout = new RolloutWidget(i18n("tool_settings.viewport.rotate_tool", "Rotate Settings"));
    auto settings = Application::instance().get_settings();
    bool expanded = settings->get("viewport.rotate_tool.ui.rotate_settings", true);
    rollout->set_expanded(expanded);
    connect(rollout, &RolloutWidget::clicked, [](bool expanded) {
        auto settings = Application::instance().get_settings();
        settings->set("viewport.rotate_tool.ui.rotate_settings", !expanded);
    });

    rollout->set_layout(content_layout);

    qobject_cast<QVBoxLayout*>(this->layout())->insertWidget(0, rollout);

    m_settings_changed_cid["viewport.rotate_tool.orientation"] = settings->register_setting_changed(
        "viewport.rotate_tool.orientation", [axis_orientation_cb](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            int ind = 0;
            if (!val.try_get<int>(&ind))
                return;
            axis_orientation_cb->setCurrentIndex(ind);
        });
    m_settings_changed_cid["viewport.rotate_tool.step"] = settings->register_setting_changed(
        "viewport.rotate_tool.step", [step_widget](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            double step = 0;
            if (!val.try_get<double>(&step))
                return;
            step_widget->setText(QString::number(step));
        });
    m_settings_changed_cid["viewport.rotate_tool.step_mode"] = settings->register_setting_changed(
        "viewport.rotate_tool.step_mode", [step_widget, enable_step_mode_cb](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            bool enable = 0;
            if (!val.try_get<bool>(&enable))
                return;
            enable_step_mode_cb->setChecked(enable);
            step_widget->setEnabled(enable_step_mode_cb->isChecked());
        });
}

ViewportRotateToolSettingsWidget::~ViewportRotateToolSettingsWidget()
{
    for (const auto& entry : m_settings_changed_cid)
        Application::instance().get_settings()->unregister_setting_changed(entry.first, entry.second);
}

OPENDCC_NAMESPACE_CLOSE
