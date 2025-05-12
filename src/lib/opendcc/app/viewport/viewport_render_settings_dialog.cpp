// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

// workaround
#include "opendcc/base/qt_python.h"

#include "opendcc/app/viewport/viewport_render_settings_dialog.h"
#include "opendcc/app/viewport/viewport_widget.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"
#include "opendcc/app/viewport/viewport_hydra_engine.h"
#include <pxr/imaging/hd/renderDelegate.h>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>
#include <QScrollArea>
#include "opendcc/ui/common_widgets/ladder_widget.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    void clear_layout(QFormLayout* layout)
    {
        while (layout->rowCount() > 0)
        {
            layout->removeRow(0);
        }
    }
}

ViewportRenderSettingsDialog::ViewportRenderSettingsDialog(ViewportWidget* viewport_widget, QWidget* parent /*= nullptr*/)
    : m_viewport_widget(viewport_widget)
    , QDialog(parent)
{
    setWindowTitle("Render Settings");
    connect(viewport_widget, &ViewportWidget::render_plugin_changed, this, &ViewportRenderSettingsDialog::on_render_plugin_changed);
    QPushButton* restore_defaults_btn = new QPushButton("Restore Defaults");
    restore_defaults_btn->setAutoDefault(false);
    connect(restore_defaults_btn, &QPushButton::clicked, this, &ViewportRenderSettingsDialog::restore_defaults);

    auto scroll_area = new QScrollArea;
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    scroll_area->setWidgetResizable(true);
    auto layout = new QVBoxLayout;
    auto content_widget = new QWidget;
    m_settings_layout = new QFormLayout(content_widget);
    m_settings_layout->setLabelAlignment(Qt::AlignmentFlag::AlignLeft);
    m_settings_layout->setFormAlignment(Qt::AlignmentFlag::AlignRight);
    scroll_area->setWidget(content_widget);
    layout->addWidget(scroll_area);
    layout->addWidget(restore_defaults_btn);
    setLayout(layout);

    update_settings();
}

void ViewportRenderSettingsDialog::on_render_plugin_changed(const PXR_NS::TfToken& render_plugin)
{
    if (render_plugin != m_current_render_plugin)
    {
        m_current_render_plugin = render_plugin;
        update_settings();
    }
}

void ViewportRenderSettingsDialog::restore_defaults()
{
    const auto& engine = m_viewport_widget->get_gl_widget()->get_engine();
    const auto& render_settings = engine->get_render_setting_descriptors();

    for (int i = 0; i < m_render_settings.size(); i++)
    {
        update_setting(m_settings_widgets[i], m_render_settings[i].key, m_render_settings[i].defaultValue);
        m_viewport_widget->get_gl_widget()->get_engine()->set_render_setting(m_render_settings[i].key, m_render_settings[i].defaultValue);
    }
}

void ViewportRenderSettingsDialog::update_settings()
{
    const auto& engine = m_viewport_widget->get_gl_widget()->get_engine();
    const auto& render_settings = engine->get_render_setting_descriptors();

    clear_layout(m_settings_layout);

    m_settings_widgets.clear();
    m_settings_widgets.reserve(m_render_settings.size());
    for (const auto& descriptor : render_settings)
    {
        const auto& value = engine->get_render_setting(descriptor.key);
        const auto& widget = create_attribute_widget(descriptor.key, value);
        if (!widget)
        {
            continue;
        }

        m_settings_layout->addRow(QString::fromStdString(descriptor.name), widget);
        m_settings_widgets.push_back(widget);
    }
    m_render_settings = render_settings;
}

QWidget* ViewportRenderSettingsDialog::create_attribute_widget(const PXR_NS::TfToken& key, const PXR_NS::VtValue& value)
{
    QWidget* widget = nullptr;
    if (value.IsHolding<bool>())
    {
        QCheckBox* check_box = new QCheckBox;
        check_box->setChecked(value.UncheckedGet<bool>());
        connect(check_box, &QCheckBox::clicked, this,
                [this, key](bool state) { m_viewport_widget->get_gl_widget()->get_engine()->set_render_setting(key, VtValue((bool)state)); });
        widget = check_box;
    }
    else if (value.IsHolding<int>())
    {
        LadderNumberWidget* number_widget = new LadderNumberWidget(nullptr, true);
        number_widget->setText(QString::number(value.UncheckedGet<int>()));
        connect(number_widget, &LadderNumberWidget::editingFinished, this, [this, key, number_widget] {
            m_viewport_widget->get_gl_widget()->get_engine()->set_render_setting(key, VtValue(number_widget->text().toInt()));
        });
        widget = number_widget;
    }
    else if (value.IsHolding<unsigned>())
    {
        LadderNumberWidget* number_widget = new LadderNumberWidget(nullptr, true);
        number_widget->setText(QString::number(value.UncheckedGet<unsigned>()));
        connect(number_widget, &LadderNumberWidget::editingFinished, this, [this, key, number_widget] {
            m_viewport_widget->get_gl_widget()->get_engine()->set_render_setting(key, VtValue(number_widget->text().toUInt()));
        });
        widget = number_widget;
    }
    else if (value.IsHolding<float>())
    {
        LadderNumberWidget* number_widget = new LadderNumberWidget(nullptr, false);
        number_widget->setText(QString::number(value.UncheckedGet<float>()));
        connect(number_widget, &LadderNumberWidget::editingFinished, this, [this, key, number_widget] {
            m_viewport_widget->get_gl_widget()->get_engine()->set_render_setting(key, VtValue(number_widget->text().toFloat()));
        });
        widget = number_widget;
    }
    else if (value.IsHolding<double>())
    {
        LadderNumberWidget* number_widget = new LadderNumberWidget(nullptr, false);
        number_widget->setText(QString::number(value.UncheckedGet<double>()));
        connect(number_widget, &LadderNumberWidget::editingFinished, this, [this, key, number_widget] {
            m_viewport_widget->get_gl_widget()->get_engine()->set_render_setting(key, VtValue(number_widget->text().toDouble()));
        });
        widget = number_widget;
    }
    else if (value.IsHolding<std::string>())
    {
        QLineEdit* line_edit = new QLineEdit;
        line_edit->setText(QString::fromStdString(value.UncheckedGet<std::string>()));
        connect(line_edit, &QLineEdit::textChanged, this, [this, key](const QString& value) {
            m_viewport_widget->get_gl_widget()->get_engine()->set_render_setting(key, VtValue(value.toStdString()));
        });
        widget = line_edit;
    }

    return widget;
}

void ViewportRenderSettingsDialog::update_setting(QWidget* widget, const PXR_NS::TfToken& key, const PXR_NS::VtValue& value)
{
    if (widget == nullptr)
    {
        return;
    }

    if (value.IsHolding<bool>())
    {
        QCheckBox* check_box = qobject_cast<QCheckBox*>(widget);
        check_box->setChecked(value.UncheckedGet<bool>());
    }
    else if (value.IsHolding<int>() || value.IsHolding<unsigned>() || value.IsHolding<float>() || value.IsHolding<double>())
    {
        LadderNumberWidget* spin_box = qobject_cast<LadderNumberWidget*>(widget);
        QString text;
        if (value.IsHolding<int>())
            text = QString::number(value.UncheckedGet<int>());
        else if (value.IsHolding<unsigned>())
            text = QString::number(value.UncheckedGet<unsigned>());
        else if (value.IsHolding<float>())
            text = QString::number(value.UncheckedGet<float>());
        else
            text = QString::number(value.UncheckedGet<double>());

        spin_box->setText(text);
    }
    else if (value.IsHolding<std::string>())
    {
        QLineEdit* line_edit = qobject_cast<QLineEdit*>(widget);
        line_edit->setText(QString::fromStdString(value.UncheckedGet<std::string>()));
    }
}

OPENDCC_NAMESPACE_CLOSE