// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/application.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QPushButton>
#include <array>
#include <QComboBox>

#include <QVBoxLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>
#include <QToolBar>
#include <QDoubleSpinBox>
#include <QActionGroup>
#include <array>

#include "opendcc/usd_editor/common_tools/viewport_select_tool_settings_widget.h"
#include "opendcc/app/viewport/tool_settings_view.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/core/application.h"
#include "opendcc/base/logging/logger.h"

#include "opendcc/usd_editor/sculpt_tool/sculpt_tool_settings.h"
#include "opendcc/usd_editor/sculpt_tool/sculpt_tool_context.h"

#include "opendcc/ui/common_widgets/ladder_widget.h"
#include "opendcc/ui/common_widgets/color_widget.h"
#include "opendcc/ui/common_widgets/rollout_widget.h"
#include "opendcc/ui/common_widgets/number_value_widget.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

REGISTER_TOOL_SETTINGS_VIEW(TfToken("Sculpt"), TfToken("USD"), SculptToolContext, SculptToolSettingsWidget);

SculptToolSettingsWidget::~SculptToolSettingsWidget()
{
    Application::instance().get_settings()->unregister_setting_changed(SculptToolContext::settings_prefix() + ".radius", m_radius_changed);
}

void SculptToolSettingsWidget::update(SculptToolContext* tool_context, bool reset /*= false*/)
{
    if (!tool_context || tool_context->empty())
    {
        setEnabled(false);
        return;
    }
    setEnabled(true);
}

SculptToolSettingsWidget::SculptToolSettingsWidget(SculptToolContext* tool_context)
{
    if (!tool_context)
    {
        OPENDCC_ERROR("Coding error: Invalid tool context.");
        return;
    }

    auto main_layout = new QVBoxLayout;
    main_layout->setContentsMargins(0, 0, 0, 0);
    auto settings = Application::instance().get_settings();
    auto mode_toolbar1 = new QToolBar;
    mode_toolbar1->setIconSize(QSize(32, 32));
    mode_toolbar1->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    auto mode_toolbar2 = new QToolBar;
    mode_toolbar2->setIconSize(QSize(32, 32));
    mode_toolbar2->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    auto mode_action_group = new QActionGroup(mode_toolbar1);

    auto add_action = [&](QToolBar* toolbar, const char* icon, std::string name, Mode mode) {
        auto action = new QAction(QIcon(icon), name.c_str(), toolbar);
        action->setCheckable(true);
        action->setData(QVariant::fromValue(static_cast<int>(mode))); // meh

        toolbar->addAction(action);
        mode_action_group->addAction(action);

        auto current_widget = toolbar->widgetForAction(action);
        current_widget->setMaximumWidth(55);
        current_widget->setMinimumWidth(55);

        if (tool_context->properties().mode == mode)
        {
            action->setChecked(true);
        }
    };

    add_action(mode_toolbar1, ":/icons/sculpt_tool_sculpt", i18n("tool_settings.Sculpt", "Sculpt").toStdString(), Mode::Sculpt);
    add_action(mode_toolbar1, ":/icons/sculpt_tool_flatten", i18n("tool_settings.Sculpt", "Flatten").toStdString(), Mode::Flatten);
    add_action(mode_toolbar1, ":/icons/sculpt_tool_move", i18n("tool_settings.Sculpt", "Move").toStdString(), Mode::Move);
    add_action(mode_toolbar1, ":/icons/sculpt_tool_noise", i18n("tool_settings.Sculpt", "Noise").toStdString(), Mode::Noise);
    add_action(mode_toolbar1, ":/icons/sculpt_tool_smooth", i18n("tool_settings.Sculpt", "Smooth").toStdString(), Mode::Smooth);
    add_action(mode_toolbar2, ":/icons/sculpt_tool_relax", i18n("tool_settings.Sculpt", "Relax").toStdString(), Mode::Relax);

    int32_t current_layout_line = 0;
    auto options_layout = new QGridLayout;

    auto mode_vertical_layout = new QVBoxLayout;
    auto mode_horyzontal_layout = new QHBoxLayout;
    mode_vertical_layout->addWidget(mode_toolbar1, 0, Qt::AlignLeft);
    mode_vertical_layout->addWidget(mode_toolbar2, 0, Qt::AlignLeft);
    mode_horyzontal_layout->addLayout(mode_vertical_layout);
    mode_horyzontal_layout->setAlignment(Qt::AlignHCenter);

    auto radius_widget = new FloatValueWidget(0.0f, FLT_MAX, 2);
    auto strength_widget = new FloatValueWidget(0.0f, FLT_MAX, 2);
    auto falloff_widget = new FloatValueWidget(0.0f, 1, 2);

    // Radius
    {
        radius_widget->set_clamp_minimum(0.f);
        radius_widget->set_value((double)tool_context->properties().radius);
        radius_widget->set_soft_range(0.f, 10.f);
        radius_widget->setEnabled(true);
        connect(radius_widget, &FloatValueWidget::editing_finished, this, [tool_context, radius_widget] {
            if (tool_context)
            {
                auto properties = tool_context->properties();
                properties.radius = radius_widget->get_value();
                tool_context->set_properties(properties);
            }
        });
        auto settings = Application::instance().get_settings();

        m_radius_changed =
            settings->register_setting_changed(SculptToolContext::settings_prefix() + ".radius.current",
                                               [radius_widget, tool_context](const std::string&, const Settings::Value& val, Settings::ChangeType) {
                                                   if (!val)
                                                       return;

                                                   const auto radius_prefix = SculptToolContext::settings_prefix() + ".radius";
                                                   const auto mode_postfix = "." + std::to_string(static_cast<int>(tool_context->properties().mode));

                                                   auto settings = Application::instance().get_settings();

                                                   radius_widget->blockSignals(true);
                                                   const float current_radius = settings->get(radius_prefix + ".current", 1.0f);
                                                   radius_widget->set_value((double)current_radius);
                                                   settings->set(radius_prefix + mode_postfix, current_radius);
                                                   radius_widget->blockSignals(false);
                                               });

        options_layout->addWidget(new QLabel(i18n("tool_settings.Sculpt", "Radius") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(radius_widget, current_layout_line++, 1);
    }

    // Strength
    {
        strength_widget->set_clamp_minimum(0.0f);
        strength_widget->set_soft_range(0.f, 10.f);
        strength_widget->set_value((double)tool_context->properties().strength);
        strength_widget->setEnabled(true);
        connect(strength_widget, &FloatValueWidget::editing_finished, this, [tool_context, strength_widget] {
            if (tool_context)
            {
                auto properties = tool_context->properties();
                properties.strength = strength_widget->get_value();
                tool_context->set_properties(properties);
            }
        });
        options_layout->addWidget(new QLabel(i18n("tool_settings.Sculpt", "Strength") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(strength_widget, current_layout_line++, 1);
    }

    // Falloff
    {
        falloff_widget->set_clamp(0.0f, 1.f);
        falloff_widget->set_soft_range(0.f, 1.f);
        falloff_widget->set_value((double)tool_context->properties().falloff);
        falloff_widget->setEnabled(true);
        connect(falloff_widget, &FloatValueWidget::editing_finished, this, [tool_context, falloff_widget] {
            if (tool_context)
            {
                auto properties = tool_context->properties();
                properties.falloff = falloff_widget->get_value();
                tool_context->set_properties(properties);
            }
        });
        options_layout->addWidget(new QLabel(i18n("tool_settings.Sculpt", "Falloff") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(falloff_widget, current_layout_line++, 1);
    }

    options_layout->setColumnStretch(0, 2);
    options_layout->setColumnStretch(1, 5);

    {
        auto modes_rollout = new RolloutWidget(i18n("tool_settings.Sculpt", "Mode"));
        bool modes_expanded = settings->get("sculpt_tool.ui.sculpt_modes", true);
        modes_rollout->set_expanded(modes_expanded);
        connect(modes_rollout, &RolloutWidget::clicked, [](bool expanded) {
            auto settings = Application::instance().get_settings();
            settings->set("sculpt_tool.ui.sculpt_modes", !expanded);
        });
        modes_rollout->set_layout(mode_horyzontal_layout);
        const int number_of_column = 5;
        const int minimum_space_for_icon = 61;
        modes_rollout->setMinimumWidth(minimum_space_for_icon * number_of_column);
        main_layout->addWidget(modes_rollout);
    }

    {
        auto options_rollout = new RolloutWidget(i18n("tool_settings.Sculpt", "Options"));
        bool options_expanded = settings->get("sculpt_tool.ui.sculpt_options", true);
        options_rollout->set_expanded(options_expanded);
        connect(options_rollout, &RolloutWidget::clicked, [](bool expanded) {
            auto settings = Application::instance().get_settings();
            settings->set("sculpt_tool.ui.sculpt_options", !expanded);
        });
        options_rollout->set_layout(options_layout);
        main_layout->addWidget(options_rollout);
    }

    update(tool_context, true);
    tool_context->set_on_mesh_changed_callback([this, tool_context]() { update(tool_context, true); });

    connect(mode_action_group, &QActionGroup::triggered, this, [tool_context, strength_widget, radius_widget, falloff_widget](QAction* action) {
        int data = action->data().toInt();
        auto mode = (Mode)data; // yep
        auto prop = tool_context->properties();
        prop.mode = mode;

        auto mode_postfix = "." + std::to_string(static_cast<int>(mode));

        auto settings = Application::instance().get_settings();

        prop.radius = settings->get(SculptToolContext::settings_prefix() + ".radius" + mode_postfix, 1.0f);
        prop.strength = settings->get(SculptToolContext::settings_prefix() + ".strength" + mode_postfix, 1.0f);
        prop.falloff = settings->get(SculptToolContext::settings_prefix() + ".falloff" + mode_postfix, 0.3f);

        strength_widget->blockSignals(true);
        radius_widget->blockSignals(true);
        falloff_widget->blockSignals(true);

        strength_widget->set_value(static_cast<double>(prop.strength));
        radius_widget->set_value(static_cast<double>(prop.radius));
        falloff_widget->set_value(static_cast<double>(prop.falloff));

        strength_widget->blockSignals(false);
        radius_widget->blockSignals(false);
        falloff_widget->blockSignals(false);

        tool_context->set_properties(prop);
    });

    setLayout(main_layout);
}
OPENDCC_NAMESPACE_CLOSE
