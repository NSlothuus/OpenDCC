/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"

#include <QWidget>

class QHBoxLayout;

OPENDCC_NAMESPACE_OPEN
class LadderNumberWidget;
class DoubleSlider;
/**
 * @brief Custom widget for handling double values
 *
 */
class OPENDCC_UI_COMMON_WIDGETS_API DoubleWidget : public QWidget
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a DoubleSlider widget.
     *
     * @param as_int Flag indicating whether the values should be treated as integers.
     * @param parent The parent widget.
     */
    DoubleWidget(bool as_int = false, QWidget* parent = nullptr);
    ~DoubleWidget() override = default;
    /**
     * @brief Gets the minimum value of the slider.
     *
     * @return The minimum value.
     */
    double get_minimum() const { return m_min; };
    /**
     * @brief Gets the maximum value of the slider.
     *
     * @return The maximum value.
     */
    double get_maximum() const { return m_max; };
    /**
     * @brief Sets the range for the slider.
     *
     * @param min The minimum value.
     * @param max The maximum value.
     */
    void set_range(double min, double max);
    /**
     * @brief Enables the slider with the specified range.
     *
     * @param min The minimum value for the slider.
     * @param max The maximum value for the slider.
     */
    void enable_slider(double min, double max);
    /**
     * @brief Gets the slider value.
     *
     * @return The value of the slider.
     */
    double get_value() const;
    /**
     * @brief Sets the value for the slider.
     *
     * @param value The value to set.
     */
    void set_value(double value);
    /**
     * @brief Sets the precision for displaying the value.
     *
     * @param precision The precision to set.
     */
    void set_precision(int precision);
    /**
     * @brief Checks if the widget values are integers.
     *
     * @return True if values are treated as integers, false otherwise.
     */
    bool is_integer() const { return m_as_int; }

Q_SIGNALS:
    /**
     * @brief Signal emitted when the value of the widget changes.
     *
     * @param value The new value of the widget.
     */
    void value_changed(double);

private:
    bool m_as_int;

    LadderNumberWidget* m_line_edit = nullptr;
    DoubleSlider* m_slider = nullptr;
    QHBoxLayout* m_layout = nullptr;

    bool m_range = false;
    double m_min, m_max;
    double m_slider_min, m_slider_max;

    bool m_use_precision = false;
    int m_precision;
};

OPENDCC_NAMESPACE_CLOSE
