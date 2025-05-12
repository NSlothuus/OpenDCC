/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include "opendcc/ui/common_widgets/ladder_widget.h"

#include <QWidget>

class QLineEdit;

OPENDCC_NAMESPACE_OPEN

class PrecisionSlider;
/**
 * @brief The FloatValueWidget class represents a custom
 * widget for displaying and editing float values.
 *
 * It provides functionality for setting the value range,
 * decimals, and handling value changes.
 */
class OPENDCC_UI_COMMON_WIDGETS_API FloatValueWidget : public QWidget
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a FloatValueWidget object with the specified parent widget.
     *
     * @param parent The parent widget.
     */
    FloatValueWidget(QWidget* parent = nullptr);
    /**
     * @brief Constructs a FloatValueWidget object with the specified
     * minimum, maximum, decimals, and parent widget.
     *
     * @param min The actual minimum value.
     * @param max The actual maximum value.
     * @param decimals The number of decimal places to display.
     * @param parent The parent widget.
     */
    FloatValueWidget(float min, float max, int decimals, QWidget* parent = nullptr);
    ~FloatValueWidget() override = default;

    /**
     * @brief Gets the number of decimal places to display.
     *
     * @return The number of decimal places.
     */
    int get_decimals() const;
    /**
     * @brief Sets the number of decimal places to display.
     *
     * @param decimals The number of decimal places to set.
     */
    void set_decimals(int decimals);
    /**
     * @brief Sets the minimum value of the widget.
     *
     * @param min The minimum value to set.
     */
    void set_soft_minimum(float min);
    /**
     * @brief Gets the minimum value of the widget.
     *
     * @return The minimum value of the widget.
     */
    float get_soft_minimum() const;
    /**
     * @brief Sets the maximum value of the widget.
     *
     * @param max The maximum value to set.
     */
    void set_soft_maximum(float max);
    /**
     * @brief Gets the maximum value of the widget.
     *
     * @return The maximum value of the widget.
     */
    float get_soft_maximum() const;
    /**
     * @brief Sets the range of the widget.
     *
     * @param min The minimum value of the range.
     * @param max The maximum value of the range.
     */
    void set_soft_range(float min, float max);
    /**
     * @brief Sets the actual minimum value of the widget.
     *
     * @param min The minimum value to set.
     */
    void set_minimum(float min);
    /**
     * @brief Gets the actual minimum value of the widget.
     *
     * @return The minimum value of the widget.
     */
    float get_minimum() const;
    /**
     * @brief Sets the actual maximum value of the widget.
     *
     * @param max The  maximum value to set.
     */
    void set_maximum(float max);
    /**
     * @brief Gets the actual maximum value of the widget.
     *
     * @return The actual maximum value of the widget.
     */
    float get_maximum() const;
    /**
     * @brief Sets the actual range and number of decimal places of the widget.
     *
     * @param min The minimum value of the range.
     * @param max The maximum value of the range.
     * @param decimals The number of decimal places to display.
     */
    void set_range(float min, float max, int decimals);
    /**
     * @brief Gets the current value of the widget.
     *
     * @return The current value of the widget.
     */
    float get_value() const;
    /**
     * @brief Sets the value of the widget.
     *
     * @param value The value to set.
     */
    void set_value(float value);
    /**
     * @brief Sets the clamp of the widget.
     *
     * @param min The minimum value of the clamp range.
     * @param max The maximum value of the clamp range.
     */
    void set_clamp(float min, float max);
    /**
     * @brief Sets the minimum value of the clamp.
     *
     * @param min The minimum value of the clamp range.
     */
    void set_clamp_minimum(float min);
    /**
     * @brief Sets the minimum value of the clamp.
     *
     * @param min The minimum value of the clamp range.
     */
    void set_clamp_maximum(float max);

Q_SIGNALS:
    /**
     * @brief Signal emitted when editing of the widget is finished.
     */
    void editing_finished();
    /**
     * @brief Signal emitted when the value of the widget changes.
     *
     * @param value The new value of the widget.
     */
    void value_changed(float);

private:
    LadderNumberWidget* m_line_edit = nullptr;
    PrecisionSlider* m_slider = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
