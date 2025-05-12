// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/common_widgets/double_widget.h"
#include "opendcc/ui/common_widgets/ladder_widget.h"
#include "opendcc/ui/common_widgets/double_slider.h"

#include <QHBoxLayout>

OPENDCC_NAMESPACE_OPEN

DoubleWidget::DoubleWidget(bool as_int /*= false*/, QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
    m_as_int = as_int;
    m_line_edit = new LadderNumberWidget(this, as_int);
    m_line_edit->setText("0");
    m_layout = new QHBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_line_edit);
    setLayout(m_layout);

    connect(m_line_edit, &LadderNumberWidget::editingFinished, [this]() {
        double value = m_line_edit->text().toDouble();
        set_value(value);
    });
}

void DoubleWidget::set_range(double min, double max)
{
    if (max < min)
        std::swap(min, max);
    m_min = min;
    m_max = max;
    m_line_edit->set_clamp(min, max);
    m_range = true;
}

void DoubleWidget::enable_slider(double min, double max)
{
    if (max < min)
        std::swap(min, max);
    min = std::max(min, m_min);
    max = std::min(max, m_max);

    m_line_edit->setFixedWidth(80);
    m_slider = new DoubleSlider;
    m_slider->set_range(min, max);
    m_slider_min = min;
    m_slider_max = max;
    m_layout->addWidget(m_slider);
    connect(m_slider, &DoubleSlider::value_changed, [this](double value) { set_value(value); });
}

double DoubleWidget::get_value() const
{
    return m_line_edit->text().toDouble();
}

void DoubleWidget::set_value(double value)
{
    if (m_range)
    {
        value = std::max(value, m_min);
        value = std::min(value, m_max);
    }

    m_line_edit->blockSignals(true);
    if (m_as_int)
    {
        m_line_edit->setText(QString::number((int)value));
    }
    else
    {
        if (!m_use_precision)
        {
            m_line_edit->setText(QString::number(value));
        }
        else
        {
            m_line_edit->setText(QString::number(value, 'f', m_precision));
        }
    }
    m_line_edit->blockSignals(false);

    if (m_slider)
    {
        m_slider->blockSignals(true);
        if (value < m_slider_min)
        {
            m_slider_min = 2 * value - m_slider_max;
            if (m_range)
            {
                m_slider_min = std::max(m_slider_min, m_min);
            }
            m_slider->set_range(m_slider_min, m_slider_max);
        }
        if (value > m_slider_max)
        {
            m_slider_max = 2 * value - m_slider_min;
            if (m_range)
            {
                m_slider_max = std::min(m_slider_max, m_max);
            }
            m_slider->set_range(m_slider_min, m_slider_max);
        }
        m_slider->set_value(value);
        m_slider->blockSignals(false);
    }

    emit value_changed(value);
}

void DoubleWidget::set_precision(int precision)
{
    m_use_precision = true;
    m_precision = precision;
}

OPENDCC_NAMESPACE_CLOSE
