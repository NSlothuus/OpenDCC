/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <QWidget>

#include "opendcc/opendcc.h"

#include "opendcc/ui/timeline_widget/range_slider.h"
#include "opendcc/ui/timeline_widget/time_widget.h"
#include "opendcc/ui/timeline_widget/frames_per_second_widget.h"

OPENDCC_NAMESPACE_OPEN

/**
 * @brief This class represents a slider widget for selecting a range of time values.
 *        It derives from QWidget and includes a RangeSlider for selecting the time range.
 */
class OPENDCC_UI_TIMELINE_WIDGET_API TimelineSlider : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a TimelineSlider.
     *
     * @param parent The parent QWidget.
     * @param f The window flags for the widget.
     */
    TimelineSlider(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /**
     * @brief Retrieves the RangeSlider widget.
     *
     * @return The RangeSlider widget.
     */
    RangeSlider* get_range_slider();

Q_SIGNALS:
    /**
     * @brief Signal emitted when the frames per second (FPS) value changes.
     *
     * @param value The new FPS value.
     */
    void fps_changed(double value);

public Q_SLOTS:
    /**
     * @brief Sets the time display mode.
     *
     * @param mode The time display mode to set.
     */
    void set_time_display(TimeDisplay mode);
    /**
     * @brief Sets the frames per second (FPS) value.
     *
     * @param fps The FPS value to set.
     */
    void set_fps(double fps);
    /**
     * @brief Updates the time widgets using m_start_time and m_end_time.
     *
     */
    void update_time_widgets(double);

private Q_SLOTS:
    void update_range_slider();

private:
    RangeSlider* m_range_slider;

    TimeWidget* m_start_time;
    TimeWidget* m_end_time;
    TimeWidget* m_current_start_time;
    TimeWidget* m_current_end_time;

    FramesPerSecondWidget* m_fps_edit;

    TimeDisplay m_time_display = TimeDisplay::Frames;
};

OPENDCC_NAMESPACE_CLOSE
