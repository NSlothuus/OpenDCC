// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>

#include <QBoxLayout>

#include "opendcc/ui/common_widgets/vec_widget.h"
#include "opendcc/ui/common_widgets/ramp_widget.h"
#include "opendcc/ui/common_widgets/tiny_slider.h"

OPENDCC_NAMESPACE_OPEN

VecWidget::VecWidget(int size, bool as_int, bool slideable, QWidget* parent)
    : QWidget(parent)
{
    auto base_layout = new QHBoxLayout;
    base_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(base_layout);

    QColor box_color[] = { { 42, 32, 32 }, { 32, 42, 32 }, { 32, 32, 42 }, { 42, 42, 42 } };

    QColor slider_color[] = { { 128, 32, 32 }, { 32, 128, 32 }, { 32, 32, 128 }, { 128, 128, 128 } };

    m_size = std::max(1, std::min(4, size));
    for (int i = 0; i < m_size; i++)
    {
        auto lay = new QVBoxLayout;
        lay->setContentsMargins(0, 0, 0, 0);
        lay->setSpacing(0);
        base_layout->addLayout(lay);

        auto flw = new FloatWidget(this);
        m_floats[i] = flw;
        lay->addWidget(flw);
        flw->setFixedHeight(15);
        flw->setSingleStep(0.1f);

        auto pal = flw->palette();
        pal.setColor(QPalette::Base, box_color[i]);
        flw->setPalette(pal);

        if (slideable)
        {
            auto slider = new HTinySlider(this);
            lay->addWidget(slider);
            if (size > 1)
                slider->slider_color(slider_color[i]);
            else
                slider->slider_color(slider_color[3]);
            slider->min(0);
            slider->max(100);
            slider->value(0);

            slider->connect(slider, SIGNAL(changing(double)), flw, SLOT(setValue(double)));
            slider->connect(flw, SIGNAL(valueChanged(double)), slider, SLOT(value(double)));

            QObject::connect(slider, &HTinySlider::changing, this, [this](double) { changed(); });

            QObject::connect(flw, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [this](double) { changed(); });
        }
    }
}

void VecWidget::value(float* val)
{
    for (int i = 0; i < m_size; i++)
        m_floats[i]->setValue(val[i]);
}

void VecWidget::value(int* val)
{
    for (int i = 0; i < m_size; i++)
        m_floats[i]->setValue(val[i]);
}

void VecWidget::get_value(int* val)
{
    for (int i = 0; i < m_size; i++)
        val[i] = m_floats[i]->value();
}

void VecWidget::get_value(float* val)
{
    for (int i = 0; i < m_size; i++)
        val[i] = m_floats[i]->value();
}
OPENDCC_NAMESPACE_CLOSE
