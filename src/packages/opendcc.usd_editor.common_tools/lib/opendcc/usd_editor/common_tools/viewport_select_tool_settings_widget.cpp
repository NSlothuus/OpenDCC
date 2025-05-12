// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include "opendcc/usd_editor/common_tools/viewport_select_tool_settings_widget.h"
#include "opendcc/app/viewport/tool_settings_view.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/core/settings.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"

#include "opendcc/ui/common_widgets/rollout_widget.h"
#include "opendcc/ui/common_widgets/ramp_widget.h"
#include "opendcc/ui/common_widgets/tiny_slider.h"
#include "opendcc/ui/common_widgets/gradient_widget.h"
#include "opendcc/ui/common_widgets/number_value_widget.h"
#include "opendcc/app/ui/application_ui.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGroupBox>
#include <QMap>
#include <array>
#include <QList>
#include <QAction>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

REGISTER_TOOL_SETTINGS_VIEW(SelectToolTokens->name, TfToken("USD"), ViewportSelectToolContext, ViewportSelectToolSettingsWidget);

ViewportSelectToolSettingsWidget::ViewportSelectToolSettingsWidget(ViewportSelectToolContext* tool_context)
    : m_tool_context(tool_context)
{
    if (!TF_VERIFY(tool_context, "Invalid tool context."))
    {
        return;
    }

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    init_common_selection_options();
    init_soft_selection();
    setLayout(m_layout);
}

ViewportSelectToolSettingsWidget::~ViewportSelectToolSettingsWidget()
{
    Application::instance().unregister_event_callback(Application::EventType::SELECTION_MODE_CHANGED, m_selection_mask_changed_cid);
    Application::instance().unregister_event_callback(Application::EventType::SELECTION_CHANGED, m_selection_changed_cid);
    for (const auto& entry : m_settings_changed_cid)
        Application::instance().get_settings()->unregister_setting_changed(entry.first, entry.second);
}

void ViewportSelectToolSettingsWidget::init_common_selection_options()
{
    static const QMap<QString, QString> icon_map = {
        { "Point", ":/icons/select_points" },         { "Edge", ":/icons/select_edges" },
        { "Face", ":/icons/select_faces" },           { "Instance", ":/icons/select_instances" },
        { "Prim", ":/icons/select_prims" },           { "model", ":/icons/select_models" },
        { "group", ":/icons/select_groups" },         { "assembly", ":/icons/select_assemblies" },
        { "component", ":/icons/select_components" }, { "subcomponent", ":/icons/select_subcomponents" },
    };

    auto rollout = new RolloutWidget(i18n("tool_settings.viewport.select_tool", "Common Selection Options"));
    auto settings = Application::instance().get_settings();
    bool expanded = settings->get("viewport.select_tool.ui.common_selection_options", true);
    rollout->set_expanded(expanded);
    connect(rollout, &RolloutWidget::clicked, [](bool expanded) {
        auto settings = Application::instance().get_settings();
        settings->set("viewport.select_tool.ui.common_selection_options", !expanded);
    });

    m_layout->addWidget(rollout);

    auto content_layout = new QGridLayout;
    content_layout->setColumnStretch(0, 2);
    content_layout->setColumnStretch(1, 5);

    auto selection_modes_cb = new QComboBox;

    auto action_group = m_tool_context->get_selection_mode_action_group();
    auto actions = action_group->actions();

    QString action_text;

    QString custom_kind_icon = ":/icons/select_components";

    for (const auto& action : actions)
    {
        action_text = action->text();

        if (icon_map.contains(action_text))
        {
            selection_modes_cb->addItem(QIcon(icon_map[action_text]), action_text, action_text);
        }
        else
        {
            selection_modes_cb->addItem(QIcon(custom_kind_icon), action_text, action_text);
        }
    }

    selection_modes_cb->setCurrentIndex(static_cast<int>(Application::instance().get_selection_mode()));

    auto label = new QLabel(i18n("tool_settings.viewport.select_tool", "Selection Mode:"));
    content_layout->addWidget(label, 0, 0, Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(selection_modes_cb, 0, 1, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);

    connect(selection_modes_cb, (void(QComboBox::*)(int)) & QComboBox::currentIndexChanged, this, [selection_modes_cb, this](int index) {
        const auto mode = static_cast<Application::SelectionMode>(index);
        const bool new_mode = mode != Application::instance().get_selection_mode();
        const auto count = static_cast<int>(Application::SelectionMode::COUNT);

        if (index < count && (new_mode || mode == Application::SelectionMode::PRIMS))
        {
            m_tool_context->set_selection_kind(TfToken());
            Application::instance().set_selection_mode(mode);
        }
        else if (index >= count)
        {
            m_tool_context->set_selection_kind(TfToken(selection_modes_cb->itemText(index).toStdString()));
            Application::instance().set_selection_mode(Application::SelectionMode::PRIMS);
        }
    });
    auto selected_mode_changed = [selection_modes_cb, this] {
        const auto mode = Application::instance().get_selection_mode();
        const auto mode_id = static_cast<int>(mode);
        const auto& kind = m_tool_context->get_selection_kind();

        if (mode_id != selection_modes_cb->currentIndex() && kind.IsEmpty())
        {
            selection_modes_cb->setCurrentIndex(mode_id);
        }
        else if (mode == Application::SelectionMode::PRIMS && !kind.IsEmpty())
        {
            const auto count = selection_modes_cb->count();

            for (int i = 0; i < count; ++i)
            {
                if (selection_modes_cb->itemText(i).toStdString() == kind)
                {
                    selection_modes_cb->setCurrentIndex(i);
                    break;
                }
            }
        }
    };

    m_selection_mask_changed_cid =
        Application::instance().register_event_callback(Application::EventType::SELECTION_MODE_CHANGED, selected_mode_changed);
    rollout->set_layout(content_layout);
}

void ViewportSelectToolSettingsWidget::init_soft_selection()
{
    auto rollout = new RolloutWidget(i18n("tool_settings.viewport.select_tool", "Soft Selection"));
    auto settings = Application::instance().get_settings();
    bool expanded = settings->get("viewport.select_tool.ui.soft_selection", true);
    rollout->set_expanded(expanded);
    connect(rollout, &RolloutWidget::clicked, [](bool expanded) {
        auto settings = Application::instance().get_settings();
        settings->set("viewport.select_tool.ui.soft_selection", !expanded);
    });

    m_layout->addWidget(rollout);
    auto content_layout = new QGridLayout;
    content_layout->setColumnStretch(0, 2);
    content_layout->setColumnStretch(1, 5);

    auto enable_soft_selection = new QCheckBox;
    enable_soft_selection->setChecked(Application::instance().is_soft_selection_enabled());
    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.select_tool", "Soft Select:")), 0, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(enable_soft_selection, 0, 1, Qt::AlignmentFlag::AlignVCenter);

    auto falloff_mode_cb = new QComboBox;
    falloff_mode_cb->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    falloff_mode_cb->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    falloff_mode_cb->addItem(i18n("tool_settings.viewport.select_tool", "Volume"));
    falloff_mode_cb->setCurrentIndex(settings->get("soft_selection.falloff_mode", 0));
    falloff_mode_cb->setEnabled(enable_soft_selection->isChecked());
    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.select_tool", "Falloff Mode:")), 1, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(falloff_mode_cb, 1, 1, Qt::AlignmentFlag::AlignVCenter);

    auto falloff_radius_widget = new FloatValueWidget(0.0f, 100000.0f, 2);
    falloff_radius_widget->set_clamp(0.0f, 100000.0f);
    falloff_radius_widget->set_soft_range(0.f, 100.f);
    falloff_radius_widget->set_value(settings->get("soft_selection.falloff_radius", 5.0f));
    connect(falloff_radius_widget, &FloatValueWidget::editing_finished, this, [falloff_radius_widget] {
        Application::instance().get_settings()->set("soft_selection.falloff_radius", falloff_radius_widget->get_value());
    });
    falloff_radius_widget->setEnabled(enable_soft_selection->isChecked());
    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.select_tool", "Falloff Radius:")), 2, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(falloff_radius_widget, 2, 1, Qt::AlignmentFlag::AlignVCenter);

    auto falloff_curve_editor = new RampEditor(m_tool_context->get_falloff_curve_ramp());
    connect(falloff_curve_editor, qOverload<>(&RampEditor::value_changed), this, [this] { m_tool_context->update_falloff_curve_ramp(); });
    falloff_curve_editor->setEnabled(enable_soft_selection->isChecked());
    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.select_tool", "Falloff Curve:")), 3, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(falloff_curve_editor, 3, 1, Qt::AlignmentFlag::AlignVCenter);

    auto viewport_color_cb = new QCheckBox();
    viewport_color_cb->setChecked(settings->get("soft_selection.enable_color", true));
    viewport_color_cb->setEnabled(enable_soft_selection->isChecked());
    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.select_tool", "Viewport Color:")), 4, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(viewport_color_cb, 4, 1, Qt::AlignmentFlag::AlignVCenter);

    auto falloff_color_editor = new GradientEditor(m_tool_context->get_falloff_color_ramp());
    connect(falloff_color_editor, &GradientEditor::end_changing, this, [this] { m_tool_context->update_falloff_color_ramp(); });
    falloff_color_editor->setEnabled(enable_soft_selection->isChecked() && viewport_color_cb->isChecked());
    content_layout->addWidget(new QLabel(i18n("tool_settings.viewport.select_tool", "Falloff Color:")), 5, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    content_layout->addWidget(falloff_color_editor, 5, 1, Qt::AlignmentFlag::AlignVCenter);

    auto toggle_soft_selection_widgets = [falloff_mode_cb, falloff_radius_widget, falloff_curve_editor, viewport_color_cb,
                                          falloff_color_editor](bool enable) {
        if (enable)
        {
            falloff_mode_cb->setEnabled(true);
            falloff_radius_widget->setEnabled(true);
            falloff_curve_editor->setEnabled(true);
            viewport_color_cb->setEnabled(true);
            falloff_color_editor->setEnabled(viewport_color_cb->isChecked());
        }
        else
        {
            falloff_mode_cb->setEnabled(false);
            falloff_radius_widget->setEnabled(false);
            falloff_curve_editor->setEnabled(false);
            viewport_color_cb->setEnabled(false);
            falloff_color_editor->setEnabled(false);
        }
    };
    m_selection_changed_cid = Application::instance().register_event_callback(
        Application::EventType::SELECTION_CHANGED, [enable_soft_selection, toggle_soft_selection_widgets] {
            const auto enabled = Application::instance().is_soft_selection_enabled();
            if (enabled == enable_soft_selection->isChecked())
                return;
            enable_soft_selection->setChecked(enabled);
            toggle_soft_selection_widgets(enabled);
        });
    m_settings_changed_cid["soft_selection.falloff_radius"] = settings->register_setting_changed(
        "soft_selection.falloff_radius", [falloff_radius_widget](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            falloff_radius_widget->set_value(val.get<float>(5.0f));
            for (const auto& viewport : ViewportWidget::get_live_widgets())
                viewport->get_gl_widget()->update();
        });
    m_settings_changed_cid["soft_selection.falloff_curve"] =
        settings->register_setting_changed("soft_selection.falloff_curve", [](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            for (const auto& viewport : ViewportWidget::get_live_widgets())
                viewport->get_gl_widget()->update();
        });
    m_settings_changed_cid["soft_selection.falloff_color"] =
        settings->register_setting_changed("soft_selection.falloff_color", [](const std::string&, const Settings::Value& val, Settings::ChangeType) {
            for (const auto& viewport : ViewportWidget::get_live_widgets())
                viewport->get_gl_widget()->update();
        });
    connect(enable_soft_selection, &QCheckBox::clicked, this, [toggle_soft_selection_widgets](bool enable) {
        Application::instance().enable_soft_selection(enable);
        toggle_soft_selection_widgets(enable);
        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });
    connect(viewport_color_cb, &QCheckBox::stateChanged, this, [falloff_color_editor](int state) {
        const bool enable = static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked;
        Application::instance().get_settings()->set("soft_selection.enable_color", enable);
        falloff_color_editor->setEnabled(enable);
        for (const auto& viewport : ViewportWidget::get_live_widgets())
            viewport->get_gl_widget()->update();
    });
    rollout->set_layout(content_layout);
}

OPENDCC_NAMESPACE_CLOSE
