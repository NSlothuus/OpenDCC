/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <vector>
#include <QWidget>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief The PrecisionSlider class represents a custom
 * widget for displaying a precision slider.
 *
 * It provides signals for value changes and slider events.
 */
class OPENDCC_UI_COMMON_WIDGETS_API PrecisionSlider : public QWidget
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a PrecisionSlider object with the specified parent widget.
     *
     * @param parent The parent widget.
     */
    PrecisionSlider(QWidget* parent = nullptr);
    ~PrecisionSlider() override;

    /**
     * @brief Sets the minimum value of the slider.
     *
     * @param min The minimum value to set.
     */
    void set_minimum(double min);
    /**
     * @brief Gets the minimum value of the slider.
     *
     * @return The minimum value of the slider.
     */
    double get_minimum() const;
    /**
     * @brief Sets the maximum value of the slider.
     *
     * @param max The maximum value to set.
     */
    void set_maximum(double max);
    /**
     * @brief Get the maximum value of the slider.
     *
     * @return The maximum value to set.
     */
    double get_maximum() const;
    /**
     * @brief Sets the range of the slider.
     *
     * @param min The minimum value of the range.
     * @param max The maximum value of the range.
     */
    void set_range(double min, double max);
    /**
     * @brief Sets the value of the slider.
     *
     * @param value The value to set.
     */
    void set_value(double value);
    /**
     * @brief Gets the current value of the slider.
     *
     * @return The current value of the slider.
     */
    double get_value() const;

    /**
     * @brief Sets whether the slider should use integer values.
     *
     * @param is_integer Whether the slider should use integer values.
     */
    void set_integer_slider(bool is_integer);
    /**
     * @brief Sets the maximum range of the slider.
     *
     * @param min The minimum value of the range.
     * @param max The maximum value of the range.
     */
    void set_autoscale_limits(double min, double max);

Q_SIGNALS:
    /**
     * @brief Signal emitted when the value of the slider changes.
     *
     * @param value The new value of the slider.
     */
    void value_changed(double);
    /**
     * @brief Signal emitted when the slider is moved.
     */
    void slider_moved();
    /**
     * @brief Signal emitted when the slider is pressed.
     */
    void slider_pressed();
    /**
     * @brief Signal emitted when the slider is released.
     */
    void slider_released();

private:
    class Handle
    {
        QRect m_rect;

    public:
        Handle(QSize const& size);

        void set_center(QPoint const& pos);
        void move_center(int const pos);
        int center_pos() const;

        QSize get_size() const;
        QRect rect() const;
    };

    struct Tick
    {
        int m_position;
        double m_value;
        QString m_str;

        Tick(double val, int pos, QString str);
        bool is_snap(int pos) const;
    };

    QRect m_scale; // a groove with ticks
    Handle* m_handle;
    std::vector<Tick*> m_ticks;

    double m_min = 0.0;
    double m_max = 1.0;
    double m_val = 0.0;
    double m_autoscale_max = std::numeric_limits<double>::max();
    double m_autoscale_min = std::numeric_limits<double>::min();
    bool m_slider_down = false;
    bool m_integer_slider = false;

    // reimplementation of QAbstractSlider (some kind of)
    int minimum() const;
    int maximum() const;
    int slider_position() const;
    void set_slider_position(int pos);
    void set_slider_down(bool is_down);

    double calc_value(int in_position) const;
    void update_slider_rect(); // update scale rect and handle position
    int convert_to_slider_position(double value) const; // m_val to pixel value
    void update_ticks(); // recalculate a number and a frequency of ticks
    int closest_tick(int pos); // get the index of a tick within m_ticks that snaps. Returns -1 if not found
    void slider_mouse_change(int pos); // mouse_move and _click common actions
    QString get_value_string(double value, bool beautify = true);

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void wheelEvent(QWheelEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* ev) override;
    virtual void mousePressEvent(QMouseEvent* ev) override;
    virtual void mouseReleaseEvent(QMouseEvent* ev) override;
    virtual void paintEvent(QPaintEvent* ev) override;
};
OPENDCC_NAMESPACE_CLOSE
