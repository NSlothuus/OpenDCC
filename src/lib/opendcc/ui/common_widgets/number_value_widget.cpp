// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/common_widgets/number_value_widget.h"
#include "opendcc/ui/common_widgets/precision_slider.h"
#include <QLineEdit>
#include <QDoubleValidator>
#include <QBoxLayout>

OPENDCC_NAMESPACE_OPEN

FloatValueWidget::FloatValueWidget(QWidget* parent /*= nullptr*/)
    : FloatValueWidget(0.0f, 1.0f, 2, parent)
{
}

FloatValueWidget::FloatValueWidget(float min, float max, int decimals, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
    if (min > max)
        std::swap(min, max);

    bool as_int = false;
    if (decimals == 0)
        as_int = true;

    m_line_edit = new LadderNumberWidget(this, as_int);
    auto validator = new QDoubleValidator(min, max, decimals);
    validator->setLocale(QLocale("English"));
    validator->setNotation(QDoubleValidator::StandardNotation);
    m_line_edit->setValidator(validator);

    m_slider = new PrecisionSlider;
    m_slider->set_autoscale_limits(min, max);
    m_slider->set_range(min, max);

    connect(m_slider, &PrecisionSlider::slider_moved, this, [this]() {
        m_line_edit->setText(QString::number(m_slider->get_value(), 'f', get_decimals()));
        editing_finished();
    });
    connect(m_line_edit, &QLineEdit::editingFinished, this, [this] {
        m_slider->set_value(m_line_edit->text().toFloat());
        editing_finished();
    });

    auto layout = new QHBoxLayout;
    layout->addWidget(m_line_edit, 1);
    layout->addWidget(m_slider, 2);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

int FloatValueWidget::get_decimals() const
{
    return qobject_cast<const QDoubleValidator*>(m_line_edit->validator())->decimals();
}

void FloatValueWidget::set_decimals(int decimals)
{
    set_range(get_minimum(), get_maximum(), decimals);
}

void FloatValueWidget::set_soft_minimum(float min)
{
    set_range(min, get_soft_maximum(), get_decimals());
}

float FloatValueWidget::get_soft_minimum() const
{
    return m_slider->get_minimum();
}

void FloatValueWidget::set_soft_maximum(float max)
{
    set_range(get_soft_minimum(), max, get_decimals());
}

float FloatValueWidget::get_soft_maximum() const
{
    return m_slider->get_maximum();
}

void FloatValueWidget::set_minimum(float min)
{
    set_range(min, get_maximum(), get_decimals());
}

float FloatValueWidget::get_minimum() const
{
    auto cur_validator = qobject_cast<const QDoubleValidator*>(m_line_edit->validator());
    return cur_validator->bottom();
}

void FloatValueWidget::set_maximum(float max)
{
    set_range(get_minimum(), max, get_decimals());
}

float FloatValueWidget::get_maximum() const
{
    auto cur_validator = qobject_cast<const QDoubleValidator*>(m_line_edit->validator());
    return cur_validator->top();
}

void FloatValueWidget::set_range(float min, float max, int decimals)
{
    if (min > max)
        std::swap(min, max);

    auto cur_validator = qobject_cast<const QDoubleValidator*>(m_line_edit->validator());
    auto validator = new QDoubleValidator(min, max, decimals);
    validator->setLocale(QLocale("English"));
    validator->setNotation(QDoubleValidator::StandardNotation);
    m_line_edit->setValidator(validator);
}

void FloatValueWidget::set_soft_range(float min, float max)
{
    const auto actual_min = get_minimum();
    const auto actual_max = get_maximum();

    if (min > max)
        std::swap(min, max);
    if (min < actual_min)
        min = actual_min;
    if (max > actual_max)
        max = actual_max;

    m_slider->set_range(min, max);
}

float FloatValueWidget::get_value() const
{
    return m_line_edit->text().toFloat();
}

void FloatValueWidget::set_value(float value)
{
    m_line_edit->setText(QString::number(value, 'f', get_decimals()));
    m_slider->set_value(value);
}

void FloatValueWidget::set_clamp(float min, float max)
{
    m_line_edit->set_clamp(min, max);
}

void FloatValueWidget::set_clamp_minimum(float min)
{
    m_line_edit->set_clamp_minimum(min);
}

void FloatValueWidget::set_clamp_maximum(float max)
{
    m_line_edit->set_clamp_maximum(max);
}

OPENDCC_NAMESPACE_CLOSE
