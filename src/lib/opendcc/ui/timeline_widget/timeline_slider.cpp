// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "timeline_slider.h"

#include <QHBoxLayout>

OPENDCC_NAMESPACE_OPEN

TimelineSlider::TimelineSlider(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    m_range_slider = new RangeSlider();

    QObject::connect(m_range_slider, &RangeSlider::start_time_changed, this, &TimelineSlider::update_time_widgets);
    QObject::connect(m_range_slider, &RangeSlider::end_time_changed, this, &TimelineSlider::update_time_widgets);
    QObject::connect(m_range_slider, &RangeSlider::current_start_time_changed, this, &TimelineSlider::update_time_widgets);
    QObject::connect(m_range_slider, &RangeSlider::current_end_time_changed, this, &TimelineSlider::update_time_widgets);

    m_start_time = new TimeWidget(m_time_display);
    m_end_time = new TimeWidget(m_time_display);
    m_current_start_time = new TimeWidget(m_time_display);
    m_current_end_time = new TimeWidget(m_time_display);
    m_fps_edit = new FramesPerSecondWidget;

    QObject::connect(m_start_time, &QDoubleSpinBox::editingFinished, this, &TimelineSlider::update_range_slider);
    QObject::connect(m_end_time, &QDoubleSpinBox::editingFinished, this, &TimelineSlider::update_range_slider);
    QObject::connect(m_current_start_time, &QDoubleSpinBox::editingFinished, this, &TimelineSlider::update_range_slider);
    QObject::connect(m_current_end_time, &QDoubleSpinBox::editingFinished, this, &TimelineSlider::update_range_slider);
    QObject::connect(m_fps_edit, &FramesPerSecondWidget::valueChanged, this, &TimelineSlider::set_fps);

    QHBoxLayout* main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);

    main_layout->addWidget(m_start_time);
    main_layout->addWidget(m_current_start_time);
    main_layout->addWidget(m_range_slider);
    main_layout->addWidget(m_current_end_time);
    main_layout->addWidget(m_end_time);
    main_layout->addWidget(m_fps_edit);

    update_time_widgets(0.0);
    update_range_slider();
}

RangeSlider* TimelineSlider::get_range_slider()
{
    return m_range_slider;
}

void TimelineSlider::set_fps(double fps)
{
    m_range_slider->set_fps(fps);

    m_start_time->set_fps(fps);
    m_end_time->set_fps(fps);
    m_current_start_time->set_fps(fps);
    m_current_end_time->set_fps(fps);

    m_fps_edit->setValue(fps);

    emit fps_changed(fps);
}

void TimelineSlider::set_time_display(TimeDisplay mode)
{
    m_time_display = mode;

    m_start_time->set_time_display(m_time_display);
    m_end_time->set_time_display(m_time_display);
    m_current_start_time->set_time_display(m_time_display);
    m_current_end_time->set_time_display(m_time_display);
    m_range_slider->set_time_display(mode);
}

void TimelineSlider::update_time_widgets(double)
{
    if (m_start_time->value() != m_range_slider->get_start_time())
    {
        m_start_time->setValue(m_range_slider->get_start_time());
    }

    if (m_end_time->value() != m_range_slider->get_end_time())
    {
        m_end_time->setValue(m_range_slider->get_end_time());
    }

    if (m_current_start_time->value() != m_range_slider->get_current_start_time())
    {
        m_current_start_time->setValue(m_range_slider->get_current_start_time());
    }

    if (m_current_end_time->value() != m_range_slider->get_current_end_time())
    {
        m_current_end_time->setValue(m_range_slider->get_current_end_time());
    }
}

void TimelineSlider::update_range_slider()
{
    if (m_start_time->value() != m_range_slider->get_start_time())
    {
        m_range_slider->set_start_time(m_start_time->value());
    }

    if (m_end_time->value() != m_range_slider->get_end_time())
    {
        m_range_slider->set_end_time(m_end_time->value());
    }

    if (m_current_start_time->value() != m_range_slider->get_current_start_time())
    {
        m_range_slider->set_current_start_time(m_current_start_time->value());
    }

    if (m_current_end_time->value() != m_range_slider->get_current_end_time())
    {
        m_range_slider->set_current_end_time(m_current_end_time->value());
    }
}

OPENDCC_NAMESPACE_CLOSE
