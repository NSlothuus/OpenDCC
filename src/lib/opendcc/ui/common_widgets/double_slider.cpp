// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/common_widgets/double_slider.h"
#include <QEvent>
#include <cmath>

OPENDCC_NAMESPACE_OPEN

DoubleSlider::DoubleSlider(QWidget* parent /*= nullptr*/)
    : QSlider(Qt::Horizontal, parent)
{
    setSingleStep(1);
    setRange(0, 100);

    connect(this, &QSlider::valueChanged, this, [this](int value) { value_changed(get_value()); });
}

void DoubleSlider::set_range(double min, double max)
{
    if (min > max)
        std::swap(min, max);

    const auto old_value = get_value();
    m_min = min;
    m_max = max;

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

void DoubleSlider::set_value(double value)
{
    if (value > m_max)
    {
        setValue(maximum());
        value = m_max;
    }
    else if (value < m_min)
    {
        setValue(minimum());
        value = m_min;
    }
    else
    {
        setValue(convert_to_int_value(value));
    }
    value_changed(value);
}

double DoubleSlider::get_value() const
{
    return m_min + static_cast<double>(this->sliderPosition() - this->minimum()) / (this->maximum() - this->minimum()) * (m_max - m_min);
}

int DoubleSlider::convert_to_int_value(double value) const
{
    return minimum() + static_cast<int>(std::round(((value - m_min) / (m_max - m_min)) * (maximum() - minimum())));
}

OPENDCC_NAMESPACE_CLOSE