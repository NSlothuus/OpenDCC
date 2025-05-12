// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/texture_paint/texture_paint_tool_settings.h"
#include "opendcc/app/core/application.h"
#include "opendcc/usd_editor/texture_paint/texture_paint_tool_context.h"
#include "opendcc/ui/common_widgets/ladder_widget.h"
#include "opendcc/app/core/settings.h"
#include <QBoxLayout>
#include <QFormLayout>
#include "opendcc/ui/common_widgets/ramp_widget.h"
#include "opendcc/ui/common_widgets/gradient_widget.h"
#include "opendcc/ui/common_widgets/color_widget.h"
#include <QCheckBox>
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include <QFileDialog>
#include "opendcc/ui/common_widgets/rollout_widget.h"
#include "opendcc/ui/common_widgets/number_value_widget.h"
#include "opendcc/usd_editor/texture_paint/brush_properties.h"
#include "opendcc/app/ui/application_ui.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

TexturePaintToolSettingsWidget::TexturePaintToolSettingsWidget(TexturePaintToolContext* tool_context)
{
    auto main_layout = new QVBoxLayout;
    auto layout = new QGridLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    m_texture_file_widget = new QLineEdit;
    auto settings = Application::instance().get_settings();
    auto brush_props = tool_context->get_brush_properties();
    int32_t current_layout_line = 0;

    auto radius_widget = new FloatValueWidget(1, 500, 0);
    radius_widget->set_soft_range(1, 100);
    radius_widget->set_clamp(1, 500);
    radius_widget->set_value(brush_props->get_radius());
    layout->addWidget(new QLabel(i18n("texture_paint", "Radius") + ":"), current_layout_line, 0,
                      Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    layout->addWidget(radius_widget, current_layout_line++, 1);
    m_radius_changed = settings->register_setting_changed(TexturePaintToolContext::settings_prefix() + ".radius",
                                                          [radius_widget, tool_context](const Settings::Value& val) {
                                                              if (!val)
                                                                  return;
                                                              auto brush_props = tool_context->get_brush_properties();
                                                              radius_widget->set_value(val.get<float>());
                                                              brush_props->set_radius(val.get<float>());
                                                          });

    const auto update_texture_file = [tool_context, this](const Settings::Value& val) {
        if (!val)
            return;

        auto base_path = val.get<std::string>();

        if (base_path.size() != 0)
        {
            m_texture_file_widget->setText(base_path.c_str());
            tool_context->set_texture_file(base_path);
            ViewportWidget::update_all_gl_widget();
        }
    };
    update_texture_file(Settings::ValueHolder(settings->get<std::string>(TexturePaintToolContext::settings_prefix() + ".texture_file", "")));
    m_path_changed = settings->register_setting_changed(TexturePaintToolContext::settings_prefix() + ".texture_file", update_texture_file);

    connect(radius_widget, &FloatValueWidget::editing_finished, this, [tool_context, radius_widget] {
        if (tool_context)
        {
            auto brush_props = tool_context->get_brush_properties();
            brush_props->set_radius(radius_widget->get_value());
        }
    });

    connect(m_texture_file_widget, &QLineEdit::editingFinished, this, [tool_context, this] {
        tool_context->set_texture_file(m_texture_file_widget->text().toLocal8Bit().toStdString());
        ViewportWidget::update_all_gl_widget();
    });
    auto open_tex_file_btn = new QPushButton(QIcon(":icons/small_open"), "");
    open_tex_file_btn->setFixedSize(16, 16);
    open_tex_file_btn->setFlat(true);
    connect(open_tex_file_btn, &QPushButton::clicked, this, [this, tool_context] {
        auto base_path =
            QFileDialog::getOpenFileName(this, i18n("texture_paint", "Select File"), QString(),
                                         "All files (*.*);;BMP (*.bmp);;JPEG (*.jpg *.jpeg);;TIFF (*.tiff *.tif *.tx);;PNG (*.png);;EXR (*.exr)"
                                         "TGA (*.tga);;HDR (*.hdr)",
                                         nullptr);
        if (!base_path.isEmpty())
        {
            m_texture_file_widget->setText(base_path);
            tool_context->set_texture_file(m_texture_file_widget->text().toLocal8Bit().toStdString());
            ViewportWidget::update_all_gl_widget();
        }

        Application::instance().get_settings()->set(TexturePaintToolContext::settings_prefix() + ".texture_file",
                                                    base_path.toLocal8Bit().toStdString());
    });

    auto texture_layout = new QGridLayout;
    texture_layout->setColumnStretch(0, 2);
    texture_layout->setColumnStretch(1, 5);
    auto file_layout = new QHBoxLayout;
    file_layout->addWidget(m_texture_file_widget);
    file_layout->addWidget(open_tex_file_btn);
    texture_layout->addWidget(new QLabel(i18n("texture_paint", "Texture File") + ":"), current_layout_line, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    texture_layout->addLayout(file_layout, current_layout_line++, 1);

    auto save_textures = new QPushButton(i18n("texture_paint", "Save"));
    connect(save_textures, &QPushButton::clicked, this, [this, tool_context] { tool_context->bake_textures(); });

    texture_layout->addWidget(new QLabel(i18n("texture_paint", "Save Textures") + ":"), current_layout_line, 0,
                              Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    texture_layout->addWidget(save_textures, current_layout_line++, 1, Qt::AlignmentFlag::AlignLeft);

    auto falloff_curve_editor = new RampEditor(brush_props->get_falloff_curve());
    connect(falloff_curve_editor, qOverload<>(&RampEditor::value_changed), this,
            [tool_context] { tool_context->get_brush_properties()->update_falloff_curve(); });
    layout->addWidget(new QLabel(i18n("texture_paint", "Falloff Curve") + ":"), current_layout_line, 0,
                      Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    layout->addWidget(falloff_curve_editor, current_layout_line++, 1);

    auto color_editor = new ColorButton(nullptr, false);
    color_editor->setFixedHeight(20);
    color_editor->setFixedWidth(30);
    color_editor->color(QColor::fromRgbF(brush_props->get_color()[0], brush_props->get_color()[1], brush_props->get_color()[2]));

    auto edits_widget_changed = [tool_context, color_editor, this] {
        if (tool_context)
        {
            auto brush_props = tool_context->get_brush_properties();
            GfVec3f color_vec(m_edits[0]->text().toFloat(), m_edits[1]->text().toFloat(), m_edits[2]->text().toFloat());
            color_editor->color(QColor::fromRgbF(m_edits[0]->text().toFloat(), m_edits[1]->text().toFloat(), m_edits[2]->text().toFloat()));
            tool_context->get_brush_properties()->set_color(color_vec);
        }
    };

    auto color_layout = new QHBoxLayout;
    color_layout->addWidget(color_editor);

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
        color_layout->addWidget(edit);
        auto brush_props = tool_context->get_brush_properties();
        edit->setText(QString::number(brush_props->get_color()[y]));
        edit->set_marker_color((y == 0 ? 255 : 0), (y == 1 ? 255 : 0), (y == 2 ? 255 : 0));
        edit->enable_marker(true);
        edit->set_clamp(0.f, 1);
        connect(edit, &LadderNumberWidget::editingFinished, edits_widget_changed);
    }

    connect(color_editor, &ColorButton::color_changed, this, [tool_context, color_editor, this] {
        auto brush_props = tool_context->get_brush_properties();
        const auto color = color_editor->color();
        GfVec3f color_vec(color.redF(), color.greenF(), color.blueF());
        m_edits[0]->setText(QString::number(brush_props->get_color()[0]));
        m_edits[1]->setText(QString::number(brush_props->get_color()[1]));
        m_edits[2]->setText(QString::number(brush_props->get_color()[2]));
        tool_context->get_brush_properties()->set_color(color_vec);
    });

    layout->addWidget(new QLabel(i18n("texture_paint", "Color") + ":"), current_layout_line, 0,
                      Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    layout->addLayout(color_layout, current_layout_line++, 1);

    auto occlude = new QCheckBox;
    occlude->setChecked(settings->get(TexturePaintToolContext::settings_prefix() + ".occlude", true));
    tool_context->set_occlude(occlude->isChecked());
    connect(occlude, &QCheckBox::stateChanged, this, [this, tool_context, occlude](int) {
        tool_context->set_occlude(occlude->isChecked());
        Application::instance().get_settings()->set(TexturePaintToolContext::settings_prefix() + ".occlude", occlude->isChecked());
    });

    auto write_to_file = new QCheckBox;
    write_to_file->setChecked(settings->get(TexturePaintToolContext::settings_prefix() + ".auto_bake", false));
    tool_context->enable_writing_to_file(write_to_file->isChecked());
    connect(write_to_file, &QCheckBox::stateChanged, this, [this, write_to_file, tool_context](int) {
        tool_context->enable_writing_to_file(write_to_file->isChecked());
        Application::instance().get_settings()->set(TexturePaintToolContext::settings_prefix() + ".auto_bake", write_to_file->isChecked());
    });

    layout->addWidget(new QLabel(i18n("texture_paint", "Occlude") + ":"), current_layout_line, 0,
                      Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    layout->addWidget(occlude, current_layout_line++, 1);
    layout->addWidget(new QLabel(i18n("texture_paint", "Auto Bake") + ":"), current_layout_line, 0,
                      Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    layout->addWidget(write_to_file, current_layout_line++, 1);

    layout->setColumnStretch(0, 2);
    layout->setColumnStretch(1, 5);

    {
        auto texture_rollout = new RolloutWidget(i18n("texture_paint", "Texture"));
        bool texture_expanded = settings->get(TexturePaintToolContext::settings_prefix() + ".texture", true);
        texture_rollout->set_expanded(texture_expanded);
        connect(texture_rollout, &RolloutWidget::clicked, [](bool expanded) {
            auto settings = Application::instance().get_settings();
            settings->set(TexturePaintToolContext::settings_prefix() + ".texture", !expanded);
        });
        texture_rollout->set_layout(texture_layout);
        main_layout->addWidget(texture_rollout);
    }

    {
        auto options_rollout = new RolloutWidget(i18n("texture_paint", "Options"));
        bool options_expanded = settings->get(TexturePaintToolContext::settings_prefix() + ".options", true);
        options_rollout->set_expanded(options_expanded);
        connect(options_rollout, &RolloutWidget::clicked, [](bool expanded) {
            auto settings = Application::instance().get_settings();
            settings->set(TexturePaintToolContext::settings_prefix() + ".options", !expanded);
        });
        options_rollout->set_layout(layout);
        main_layout->addWidget(options_rollout);
    }

    setLayout(main_layout);
}

TexturePaintToolSettingsWidget::~TexturePaintToolSettingsWidget()
{
    Application::instance().get_settings()->unregister_setting_changed(TexturePaintToolContext::settings_prefix() + ".radius", m_radius_changed);
    Application::instance().get_settings()->unregister_setting_changed(TexturePaintToolContext::settings_prefix() + ".texture_file", m_path_changed);
}

OPENDCC_NAMESPACE_CLOSE
