// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/point_instancer_tool/tool_settings.h"

#include "opendcc/app/core/application.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QPushButton>
#include <array>
#include <QComboBox>

#include "opendcc/usd_editor/common_tools/viewport_select_tool_settings_widget.h"
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
#include "opendcc/app/core/application.h"
#include "opendcc/app/viewport/tool_settings_view.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/ui/common_widgets/ladder_widget.h"
#include "opendcc/ui/common_widgets/rollout_widget.h"
#include "opendcc/ui/common_widgets/number_value_widget.h"
#include "opendcc/usd_editor/point_instancer_tool/tool_context.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

REGISTER_TOOL_SETTINGS_VIEW(TfToken("PointInstancer"), TfToken("USD"), PointInstancerToolContext, PointInstancerToolSettingsWidget);

PointInstancerToolSettingsWidget::~PointInstancerToolSettingsWidget()
{
    Application::instance().get_settings()->unregister_setting_changed("point_instancer.properties.radius", m_radius_changed);
}

void PointInstancerToolSettingsWidget::update_type_combo_box(PointInstancerToolContext* tool_context, bool reset)
{
    if (m_type_combo_box)
        m_type_combo_box->clear();
    else
    {
        m_type_combo_box = new QComboBox;
        connect(m_type_combo_box, (void(QComboBox::*)(int)) & QComboBox::currentIndexChanged, this, [tool_context, this](int i) {
            if (tool_context)
            {
                auto prop = tool_context->properties();
                prop.current_proto_idx = i;
                tool_context->set_properties(prop);
            }
        });
    }

    if (!tool_context || !tool_context->instancer())
    {
        this->setEnabled(false);
        return;
    }
    this->setEnabled(true);
    SdfPathVector targets;
    tool_context->instancer().GetPrototypesRel().GetTargets(&targets);

    for (auto& t : targets)
        m_type_combo_box->addItem(t.GetString().c_str());

    if (reset)
        m_type_combo_box->setCurrentIndex(0);
}

PointInstancerToolSettingsWidget::PointInstancerToolSettingsWidget(PointInstancerToolContext* tool_context)
{
    if (!TF_VERIFY(tool_context, "pointInstancer: Invalid tool context."))
        return;

    auto main_layout = new QVBoxLayout;
    auto options_layout = new QGridLayout;
    options_layout->setColumnStretch(0, 2);
    options_layout->setColumnStretch(1, 5);
    int32_t current_layout_line = 0;

    auto add_prototypes_bt = new QPushButton(i18n("tool_settings.PointInstancer", "Add Selected Items as Prototypes"));
    auto add_prototypes_layout = new QHBoxLayout;
    add_prototypes_layout->addWidget(add_prototypes_bt, 1, Qt::AlignLeft);
    options_layout->addLayout(add_prototypes_layout, current_layout_line++, 1);

    connect(add_prototypes_bt, &QPushButton::released, this, [tool_context, this] {
        tool_context->add_selected_items();
        update_type_combo_box(tool_context);
    });

    update_type_combo_box(tool_context);
    {
        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Current Prototype") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(m_type_combo_box, current_layout_line++, 1, Qt::AlignmentFlag::AlignVCenter);
        m_type_combo_box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        m_type_combo_box->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

        auto mode_combo_box = new QComboBox;
        mode_combo_box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        mode_combo_box->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
        mode_combo_box->addItem(i18n("tool_settings.PointInstancer", "One"));
        mode_combo_box->addItem(i18n("tool_settings.PointInstancer", "RandomInRadius"));
        mode_combo_box->setCurrentIndex((int)tool_context->properties().mode);

        connect(mode_combo_box, (void(QComboBox::*)(int)) & QComboBox::currentIndexChanged, this, [tool_context, this](int i) {
            if (tool_context)
            {
                auto prop = tool_context->properties();
                prop.mode = (PointInstancerToolContext::Mode)i;
                tool_context->set_properties(prop);
            }
        });
        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Mode") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(mode_combo_box, current_layout_line++, 1, Qt::AlignmentFlag::AlignVCenter);
    }

    {
        auto rotate_to_normal_widget = new QCheckBox();
        rotate_to_normal_widget->setChecked(tool_context->properties().rotate_to_normal);
        connect(rotate_to_normal_widget, &QCheckBox::stateChanged, this, [tool_context, rotate_to_normal_widget] {
            if (tool_context)
            {
                auto prop = tool_context->properties();
                prop.rotate_to_normal = rotate_to_normal_widget->isCheckable();
                tool_context->set_properties(prop);
            }
        });
        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Rotate to Normal") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(rotate_to_normal_widget, current_layout_line++, 1, Qt::AlignmentFlag::AlignVCenter);
    }

    {
        std::vector<LadderNumberWidget*> scale_widgets(3, nullptr);

        auto scale_layout = new QHBoxLayout;

        for (size_t i = 0; i < 3; ++i)
        {
            scale_widgets[i] = new LadderNumberWidget(nullptr, false);
            scale_widgets[i]->setText(QString::number(tool_context->properties().scale[i]));
            scale_widgets[i]->setEnabled(true);
            scale_widgets[i]->setMaximumWidth(62);
            scale_layout->addWidget(scale_widgets[i]);
        }

        auto update_scale = [tool_context, scale_widgets] {
            if (tool_context)
            {
                auto prop = tool_context->properties();
                for (size_t i = 0; i < 3; ++i)
                    prop.scale[i] = scale_widgets[i]->text().toDouble();
                tool_context->set_properties(prop);
            }
        };

        for (size_t i = 0; i < 3; ++i)
            connect(scale_widgets[i], &LadderNumberWidget::editingFinished, this, update_scale);

        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Scale") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addLayout(scale_layout, current_layout_line++, 1, Qt::AlignmentFlag::AlignVCenter);
    }

    {
        std::vector<LadderNumberWidget*> scale_random_widgets(3, nullptr);

        auto scale_random_layout = new QHBoxLayout;

        for (size_t i = 0; i < 3; ++i)
        {
            scale_random_widgets[i] = new LadderNumberWidget(nullptr, false);
            scale_random_widgets[i]->setText(QString::number(tool_context->properties().scale_randomness[i]));
            scale_random_widgets[i]->setEnabled(true);
            scale_random_widgets[i]->setMaximumWidth(62);
            scale_random_layout->addWidget(scale_random_widgets[i]);
        }

        auto update_scale = [tool_context, scale_random_widgets] {
            if (tool_context)
            {
                auto prop = tool_context->properties();
                for (size_t i = 0; i < 3; ++i)
                    prop.scale_randomness[i] = scale_random_widgets[i]->text().toDouble();
                tool_context->set_properties(prop);
            }
        };

        for (size_t i = 0; i < 3; ++i)
            connect(scale_random_widgets[i], &LadderNumberWidget::editingFinished, this, update_scale);

        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Scale Randomness") + ", %: "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addLayout(scale_random_layout, current_layout_line++, 1, Qt::AlignmentFlag::AlignVCenter);
    }

    {
        auto rotation_layout = new QHBoxLayout;
        auto rotation_min_widget = new LadderNumberWidget(nullptr, false);
        rotation_min_widget->set_clamp(-180, 180);
        rotation_min_widget->enable_clamp(true);
        rotation_min_widget->setText(QString::number(tool_context->properties().rotation_min));
        rotation_min_widget->setEnabled(true);
        rotation_min_widget->setMaximumWidth(55);

        auto rotation_max_widget = new LadderNumberWidget(nullptr, false);
        rotation_max_widget->set_clamp(-180, 180);
        rotation_max_widget->enable_clamp(true);
        rotation_max_widget->setText(QString::number(tool_context->properties().rotation_max));
        rotation_max_widget->setEnabled(true);
        rotation_max_widget->setMaximumWidth(55);

        auto update_scale = [tool_context, rotation_min_widget, rotation_max_widget] {
            if (tool_context)
            {
                auto properties = tool_context->properties();
                properties.rotation_min = rotation_min_widget->text().toDouble();
                properties.rotation_max = rotation_max_widget->text().toDouble();
                tool_context->set_properties(properties);
            }
        };

        connect(rotation_max_widget, &LadderNumberWidget::editingFinished, this, update_scale);
        connect(rotation_min_widget, &LadderNumberWidget::editingFinished, this, update_scale);

        auto l_min = new QLabel(i18n("tool_settings.PointInstancer", "min") + ":");
        l_min->setMaximumWidth(31);

        auto l_max = new QLabel(i18n("tool_settings.PointInstancer", "max") + ":");
        l_max->setMaximumWidth(31);
        rotation_layout->addWidget(l_min);
        rotation_layout->addWidget(rotation_min_widget);
        rotation_layout->addWidget(l_max);
        rotation_layout->addWidget(rotation_max_widget);

        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Rotation, deg") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addLayout(rotation_layout, current_layout_line++, 1, Qt::AlignmentFlag::AlignVCenter);
    }

    {
        auto bend_widget = new FloatValueWidget(-FLT_MAX, FLT_MAX, 2);
        bend_widget->set_soft_range(-90.f, 90.f);
        bend_widget->set_value((double)tool_context->properties().vertical_offset);
        bend_widget->setEnabled(true);
        connect(bend_widget, &FloatValueWidget::editing_finished, this, [tool_context, bend_widget] {
            if (tool_context)
            {
                auto properties = tool_context->properties();
                properties.bend_randomness = bend_widget->get_value();
                tool_context->set_properties(properties);
            }
        });
        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Bend Randomness, deg") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(bend_widget, current_layout_line++, 1);
    }

    {
        auto vertical_offset_widget = new FloatValueWidget(-FLT_MAX, FLT_MAX, 2);
        vertical_offset_widget->set_value((double)tool_context->properties().vertical_offset);
        vertical_offset_widget->set_soft_range(-90.f, 90.f);
        vertical_offset_widget->setEnabled(true);
        connect(vertical_offset_widget, &FloatValueWidget::editing_finished, this, [tool_context, vertical_offset_widget] {
            if (tool_context)
            {
                auto properties = tool_context->properties();
                properties.vertical_offset = vertical_offset_widget->get_value();
                tool_context->set_properties(properties);
            }
        });
        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Vertical Offset") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(vertical_offset_widget, current_layout_line++, 1);
    }

    {
        auto radius_widget = new FloatValueWidget(0.0f, FLT_MAX, 2);
        radius_widget->set_clamp_minimum(0.f);
        radius_widget->set_value((double)tool_context->properties().radius);
        radius_widget->set_soft_range(0.f, 20.f);
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

        m_radius_changed = settings->register_setting_changed(PointInstancerToolContext::settings_prefix() + ".radius",
                                                              [radius_widget, tool_context](const Settings::Value& val) {
                                                                  float v;
                                                                  if (!val.try_get<float>(&v))
                                                                      return;
                                                                  auto properties = tool_context->properties();
                                                                  properties.radius = v;
                                                                  tool_context->set_properties(properties);
                                                                  radius_widget->set_value(v);
                                                              });

        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Radius") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(radius_widget, current_layout_line++, 1);
    }
    {
        auto density_widget = new FloatValueWidget(0.0f, FLT_MAX, 2);
        density_widget->set_clamp_minimum(0.f);
        density_widget->set_soft_range(0.f, 10);
        density_widget->set_value((double)tool_context->properties().density);
        density_widget->setEnabled(true);
        connect(density_widget, &FloatValueWidget::editing_finished, this, [tool_context, density_widget] {
            if (tool_context)
            {
                auto properties = tool_context->properties();
                properties.density = density_widget->get_value();
                tool_context->set_properties(properties);
            }
        });
        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Density") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(density_widget, current_layout_line++, 1);
    }
    {
        auto falloff_widget = new FloatValueWidget(0.0f, 1, 2);
        falloff_widget->set_value((double)tool_context->properties().falloff);
        falloff_widget->set_clamp(0.0f, 1);
        falloff_widget->set_soft_range(0.f, 1);
        falloff_widget->setEnabled(true);
        connect(falloff_widget, &FloatValueWidget::editing_finished, this, [tool_context, falloff_widget] {
            if (tool_context)
            {
                auto properties = tool_context->properties();
                properties.falloff = falloff_widget->get_value();
                tool_context->set_properties(properties);
            }
        });
        options_layout->addWidget(new QLabel(i18n("tool_settings.PointInstancer", "Falloff") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(falloff_widget, current_layout_line++, 1);
    }

    {
        auto settings = Application::instance().get_settings();
        auto options_rollout = new RolloutWidget(i18n("tool_settings.PointInstancer", "Options"));
        bool options_expanded = settings->get("instancer.instancer_tool.ui.instancer_options", true);
        options_rollout->set_expanded(options_expanded);
        connect(options_rollout, &RolloutWidget::clicked, [](bool expanded) {
            auto settings = Application::instance().get_settings();
            settings->set("instancer.instancer_tool.ui.instancer_options", !expanded);
        });
        options_rollout->set_layout(options_layout);
        main_layout->addWidget(options_rollout);
    }

    setLayout(main_layout);

    tool_context->set_on_instancer_changed_callback([this, tool_context]() { update_type_combo_box(tool_context, true); });
}
OPENDCC_NAMESPACE_CLOSE
