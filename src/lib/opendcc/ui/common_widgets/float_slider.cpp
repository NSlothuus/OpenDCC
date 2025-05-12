// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/common_widgets/float_slider.h"
#include <QResizeEvent>
#include <QWheelEvent>
#include <cmath>

OPENDCC_NAMESPACE_OPEN

const int FloatSlider::s_step_size = 1;

FloatSlider::FloatSlider(Qt::Orientation orientation /*= Qt::Horizontal*/, QWidget* parent /*= nullptr*/)
    : QSlider(orientation, parent)
{
    setSingleStep(1);
    setRange(0, width() / s_step_size);

    connect(this, &QSlider::valueChanged, this, [this](int value) { value_changed(get_value()); });
}

void FloatSlider::set_minimum(float min)
{
    set_range(min, m_max);
}

float FloatSlider::get_minimum() const
{
    return m_min;
}

void FloatSlider::set_maximum(float max)
{
    set_range(m_min, max);
}

float FloatSlider::get_maximum() const
{
    return m_max;
}

void FloatSlider::set_range(float min, float max)
{
    if (min > max)
        std::swap(min, max);

    const auto old_value = get_value();
    if (min > max)
    {
        m_min = max;
        m_max = min;
    }
    else
    {
        m_min = min;
        m_max = max;
    }

    if (old_value >= m_max)
        setValue(maximum());
    else if (old_value <= m_min)
        setValue(minimum());
    else
    {
        const float t = (old_value - m_min) / (m_max - m_min);
        setValue(minimum() + t * (maximum() - minimum()));
    }
}

void FloatSlider::set_value(float value)
{
    if (value > m_max)
    {
        setValue((maximum() - minimum()) / 2);
        m_max = 2 * value - m_min;
    }
    else if (value < m_min)
    {
        setValue((maximum() - minimum()) / 2);
        m_min = 2 * value - m_max;
    }
    else
    {
        setValue(convert_to_int_value(value));
    }
    value_changed(value);
}

float FloatSlider::get_value() const
{
    return m_min + static_cast<float>(this->sliderPosition() - this->minimum()) / (this->maximum() - this->minimum()) * (m_max - m_min);
}

void FloatSlider::resizeEvent(QResizeEvent* event)
{
    QSlider::resizeEvent(event);
    setRange(0, event->size().width() / s_step_size);
}

void FloatSlider::wheelEvent(QWheelEvent* e)
{
    e->ignore();
}

int FloatSlider::convert_to_int_value(float value) const
{
    return minimum() + static_cast<int>(std::round(((value - m_min) / (m_max - m_min)) * (maximum() - minimum())));
}

OPENDCC_NAMESPACE_CLOSE
