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
#include "opendcc/usd_editor/paint_primvar_tool/paint_primvar_tool_settings.h"
#include "opendcc/usd_editor/paint_primvar_tool/paint_primvar_tool_context.h"
#include "opendcc/ui/common_widgets/color_widget.h"
#include "opendcc/ui/common_widgets/rollout_widget.h"
#include "opendcc/ui/common_widgets/number_value_widget.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

REGISTER_TOOL_SETTINGS_VIEW(TfToken("PaintPrimvar"), TfToken("USD"), PaintPrimvarToolContext, PaintPrimvarToolSettingsWidget);

PaintPrimvarToolSettingsWidget::~PaintPrimvarToolSettingsWidget()
{
    Application::instance().get_settings()->unregister_setting_changed(PaintPrimvarToolContext::settings_prefix() + ".radius", m_radius_changed);
}

void PaintPrimvarToolSettingsWidget::update(PaintPrimvarToolContext* tool_context, bool reset /*= false*/)
{
    if (!m_primvar_combo_box)
        return;

    m_primvar_combo_box->clear();

    if (!tool_context || tool_context->get_primvar_type() == PrimvarType::None)
    {
        setEnabled(false);
        return;
    }
    setEnabled(true);

    auto primvars = tool_context->primvars_names();
    for (auto name : primvars)
        m_primvar_combo_box->addItem(name.GetString().c_str());

    if (reset)
        m_primvar_combo_box->setCurrentIndex(0);
}

PaintPrimvarToolSettingsWidget::PaintPrimvarToolSettingsWidget(PaintPrimvarToolContext* tool_context)
{
    if (!TF_VERIFY(tool_context, "PaintPrimvar: Invalid tool context."))
        return;

    auto main_layout = new QVBoxLayout;

    int32_t current_layout_line = 0;
    auto options_layout = new QGridLayout;
    auto settings = Application::instance().get_settings();

    auto mode_layout = new QHBoxLayout();
    {
        auto mode_combo_box = new QComboBox(this);
        mode_combo_box->addItem(i18n("tool_settings.PaintPrimvar", "Set"), int(PaintPrimvarToolContext::Mode::Set));
        mode_combo_box->addItem(i18n("tool_settings.PaintPrimvar", "Add"), int(PaintPrimvarToolContext::Mode::Add));
        mode_combo_box->addItem(i18n("tool_settings.PaintPrimvar", "Smooth"), int(PaintPrimvarToolContext::Mode::Smooth));
        mode_combo_box->setCurrentIndex((int)tool_context->properties().mode);
        mode_combo_box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        mode_combo_box->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
        connect(mode_combo_box, (void(QComboBox::*)(int)) & QComboBox::currentIndexChanged, this, [mode_combo_box, tool_context, this](int i) {
            auto prop = tool_context->properties();
            prop.mode = (PaintPrimvarToolContext::Mode)mode_combo_box->itemData(i).toInt();
            tool_context->set_properties(prop);
        });

        mode_layout->addWidget(mode_combo_box);
        options_layout->addWidget(new QLabel(i18n("tool_settings.PaintPrimvar", "Mode") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addLayout(mode_layout, current_layout_line++, 1, Qt::AlignmentFlag::AlignVCenter);
    }

    {
        m_primvar_combo_box = new QComboBox;
        m_primvar_combo_box->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        m_primvar_combo_box->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
        connect(m_primvar_combo_box, (void(QComboBox::*)(int)) & QComboBox::currentIndexChanged, this, [tool_context, this](int i) {
            if (tool_context)
            {
                tool_context->set_primvar_index(i);
                if (tool_context->get_primvar_type() == PrimvarType::Float)
                {
                    for (auto* edit : m_edits)
                        edit->setVisible(false);
                    float_value_widget->setVisible(true);
                    vec3f_value_widget->setVisible(false);
                }
                else
                {
                    for (auto* edit : m_edits)
                        edit->setVisible(true);
                    vec3f_value_widget->setVisible(true);
                    float_value_widget->setVisible(false);
                }
            }
        });
        options_layout->addWidget(new QLabel(i18n("tool_settings.PaintPrimvar", "Primvar") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(m_primvar_combo_box, current_layout_line++, 1, Qt::AlignmentFlag::AlignVCenter);
    }

    auto value_layout = new QHBoxLayout();
    {
        auto color_widget = new ColorButton(this);
        color_widget->setFixedHeight(20);
        color_widget->setFixedWidth(30);
        auto value = tool_context->properties().vec3f_value;

        QColor color(value[0] * 255.0f, value[1] * 255.0f, value[2] * 255.0f);
        color_widget->color(color);

        auto color_widget_changed = [tool_context, color_widget, this] {
            if (tool_context)
            {
                float scale = 1.0f / 255.0f;
                QColor color = color_widget->color();
                auto prop = tool_context->properties();
                prop.vec3f_value[0] = color.red() * scale;
                prop.vec3f_value[1] = color.green() * scale;
                prop.vec3f_value[2] = color.blue() * scale;

                for (size_t i = 0; i < 3; ++i)
                    m_edits[i]->setText(QString::number(prop.vec3f_value[i], 'g', int(std::log10(1 + std::abs(prop.vec3f_value[i])) + 5)));

                tool_context->set_properties(prop);
            }
        };

        auto edits_widget_changed = [tool_context, color_widget, this] {
            if (tool_context)
            {
                float scale = 255.0f;
                auto prop = tool_context->properties();
                for (size_t i = 0; i < 3; ++i)
                    prop.vec3f_value[i] = m_edits[i]->text().toFloat();
                QColor color(prop.vec3f_value[0] * scale, prop.vec3f_value[1] * scale, prop.vec3f_value[2] * scale);
                color_widget->color(color);
                tool_context->set_properties(prop);
            }
        };

        connect(color_widget, &ColorButton::color_changed, this, color_widget_changed);

        vec3f_value_widget = color_widget;
        value_layout->addWidget(vec3f_value_widget);

        for (int y = 0; y < 3; ++y)
        {
            auto edit = new LadderNumberWidget(this, false);
            m_edits.push_back(edit);
            QValidator* validator;
            auto double_validator = new QDoubleValidator(0.f, 1, 5, this);
            double_validator->setLocale(QLocale("English"));
            double_validator->setNotation(QDoubleValidator::StandardNotation);
            validator = double_validator;

            edit->setValidator(validator);
            value_layout->addWidget(edit);
            auto prop = tool_context->properties();
            std::vector<float> v = { prop.vec3f_value[0], prop.vec3f_value[1], prop.vec3f_value[2] };
            edit->setText(QString::number(prop.vec3f_value[y], 'g', int(std::log10(1 + std::abs(prop.vec3f_value[y])) + 5)));
            edit->set_marker_color((y == 0 ? 255 : 0), (y == 1 ? 255 : 0), (y == 2 ? 255 : 0));
            edit->enable_marker(true);
            edit->set_clamp(0.f, 1);
            connect(edit, &LadderNumberWidget::editingFinished, edits_widget_changed);
        }
    }

    {
        float_value_widget = new LadderNumberWidget(nullptr, false);
        float_value_widget->setVisible(false);
        float_value_widget->set_clamp(0.0f, 100);
        float_value_widget->enable_clamp(true);
        float_value_widget->setText(QString::number(tool_context->properties().float_value));
        float_value_widget->setEnabled(true);
        connect(float_value_widget, &LadderNumberWidget::editingFinished, this, [tool_context, this] {
            if (tool_context)
            {
                auto properties = tool_context->properties();
                properties.float_value = float_value_widget->text().toDouble();
                tool_context->set_properties(properties);
            }
        });
        auto settings = Application::instance().get_settings();
        value_layout->addWidget(float_value_widget);
    }

    options_layout->addWidget(new QLabel(i18n("tool_settings.PaintPrimvar", "Value") + ": "), current_layout_line, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    options_layout->addLayout(value_layout, current_layout_line++, 1);

    auto radius_widget = new FloatValueWidget(0.0f, 100, 2);
    auto falloff_widget = new FloatValueWidget(0.0f, 2, 2);

    {
        radius_widget->set_clamp(0.0f, 100);
        radius_widget->set_value(tool_context->properties().radius);
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

        m_radius_changed = settings->register_setting_changed(PaintPrimvarToolContext::settings_prefix() + ".radius",
                                                              [radius_widget, tool_context](const Settings::Value& val) {
                                                                  float v;
                                                                  if (!val.try_get<float>(&v))
                                                                      return;
                                                                  auto properties = tool_context->properties();
                                                                  properties.radius = v;
                                                                  tool_context->set_properties(properties);
                                                                  radius_widget->set_value(v);
                                                              });

        options_layout->addWidget(new QLabel(i18n("tool_settings.PaintPrimvar", "Radius") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(radius_widget, current_layout_line++, 1);
    }

    {
        falloff_widget->set_clamp(0.0f, 2);
        falloff_widget->set_value(tool_context->properties().falloff);
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
        options_layout->addWidget(new QLabel(i18n("tool_settings.PaintPrimvar", "Falloff") + ": "), current_layout_line, 0,
                                  Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
        options_layout->addWidget(falloff_widget, current_layout_line++, 1);
    }

    options_layout->setColumnStretch(0, 2);
    options_layout->setColumnStretch(1, 5);

    {
        auto options_rollout = new RolloutWidget(i18n("tool_settings.PaintPrimvar", "Options"));
        bool options_expanded = settings->get("paintprimvar_tool.ui.paintprimvar_options", true);
        options_rollout->set_expanded(options_expanded);
        connect(options_rollout, &RolloutWidget::clicked, [](bool expanded) {
            auto settings = Application::instance().get_settings();
            settings->set("paintprimvar_tool.ui.paintprimvar_options", !expanded);
        });
        options_rollout->set_layout(options_layout);
        main_layout->addWidget(options_rollout);
    }

    main_layout->addLayout(options_layout);

    update(tool_context, true);
    tool_context->set_on_mesh_changed_callback([this, tool_context]() { update(tool_context, true); });

    setLayout(main_layout);
}

OPENDCC_NAMESPACE_CLOSE
