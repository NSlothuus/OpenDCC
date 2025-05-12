/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <QWidget>

#include "opendcc/opendcc.h"
#include "opendcc/ui/timeline_widget/time_display.h"

OPENDCC_NAMESPACE_OPEN

/**
 * @class RangeSlider
 * @brief This class represents a slider widget for selecting a range of time values.
 */
class OPENDCC_UI_TIMELINE_WIDGET_API RangeSlider : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Constructs a RangeSlider.
     *
     * @param parent The parent QWidget.
     * @param f Additional window flags.
     */
    RangeSlider(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    /**
     * @brief Retrieves the start time value of the range.
     *
     * @return The start time value.
     */
    double get_start_time() const;
    /**
     * @brief Retrieves the end time value of the range.
     *
     * @return The end time value.
     */
    double get_end_time() const;

    /**
     * @brief Retrieves the current start time value of the range.
     *
     * @return The current start time value.
     */
    double get_current_start_time() const;
    /**
     * @brief Retrieves the current end time value of the range.
     *
     * @return The current end time value.
     */
    double get_current_end_time() const;

    /**
     * @brief Checks if the slider is currently being moved.
     *
     * @return True if the slider is moving, false otherwise.
     */
    bool slider_moving() const;

public Q_SLOTS:
    /**
     * @brief Sets the start time value of the range.
     *
     * @param start_time The start time value to set.
     */
    void set_start_time(double start_time);
    /**
     * @brief Sets the end time value of the range.
     *
     * @param end_time The end time value to set.
     */
    void set_end_time(double end_time);
    /**
     * @brief Sets the current start time value of the range.
     *
     * @param current_start_time The current start time value to set.
     */
    void set_current_start_time(double current_start_time);
    /**
     * @brief Sets the current end time value of the range.
     *
     * @param current_end_time The current end time value to set.
     */
    void set_current_end_time(double current_end_time);
    /**
     * @brief Sets the time display mode of the range slider.
     *
     * @param mode The TimeDisplay mode to set.
     */
    void set_time_display(TimeDisplay mode);
    /**
     * @brief Sets the frames per second (FPS) value for time calculations.
     *
     * @param fps The frames per second value to set.
     */
    void set_fps(double fps);

Q_SIGNALS:
    /**
     * @brief Signal emitted when the start time value of the range changes.
     *
     * @param start_time The new start time value.
     */
    void start_time_changed(double start_time);
    /**
     * @brief Signal emitted when the end time value of the range changes.
     *
     * @param end_time The new end time value.
     */
    void end_time_changed(double end_time);
    /**
     * @brief Signal emitted when the current start time value of the range changes.
     *
     * @param start_time The new current start time value.
     */
    void current_start_time_changed(double start_time);
    /**
     * @brief Signal emitted when the current end time value of the range changes.
     *
     * @param end_time The new current end time value.
     */
    void current_end_time_changed(double end_time);
    /**
     * @brief Signal emitted when the range values change.
     *
     * @param start_time The new start time value.
     * @param end_time The new end time value.
     */
    void range_changed(double start_time, double end_time);

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    enum class ElementType : quint8
    {
        StartSlider = 0,
        EndSlider = 1,
        VisibleTimeline = 2,
        None = 3,
    };

    void paint_background();
    void paint_visible_timeline();
    void paint_sliders();
    void paint_sliders_value();

    void update_elements();

    ElementType get_selected_element(const QPoint& pos);

    QRectF get_element_rect(const ElementType type);

    double compute_slider_size();

    double x_by_time(const double time);
    double time_by_x(const double x);

    double compute_step();

    double compute_slider_width();
    double compute_slider_height();
    double compute_visible_timeline_width();
    double compute_visible_timeline_height();

    double compute_slider_y_pos();

    double m_start_time = 1;
    double m_end_time = 24;

    double m_previous_start_time = m_start_time;
    double m_previous_end_time = m_end_time;

    double m_current_start_time = m_start_time;
    double m_current_end_time = m_end_time;

    ElementType m_selected_element = ElementType::None;

    QPoint m_first_click_pos_in_rect;
    QPoint m_current_pos;
    QPoint m_previous_pos;

    TimeDisplay m_time_display = TimeDisplay::Frames;
    double m_fps = 24.0;

    bool m_slider_moving = false;

    QColor m_background_color;
    QColor m_visible_timeline_color;
    QColor m_slider_color;
    QColor m_selected_slider_color;
    QColor m_text_color;
};

OPENDCC_NAMESPACE_CLOSE
