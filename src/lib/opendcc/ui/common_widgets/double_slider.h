/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"

#include <QSlider>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief This class provides a custom slider for double values.
 *
 */
class OPENDCC_UI_COMMON_WIDGETS_API DoubleSlider : public QSlider
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a DoubleSlider widget.
     *
     * @param parent The parent widget.
     */
    DoubleSlider(QWidget* parent = nullptr);
    ~DoubleSlider() override = default;

    /**
     * @brief Gets the minimum value.
     *
     * @return The minimum value.
     */
    double get_minimum() const { return m_min; };
    /**
     * @brief Gets the maximum value.
     *
     * @return The maximum value.
     */
    double get_maximum() const { return m_max; };
    /**
     * @brief Sets the range of the slider.
     *
     * @param min The minimum value.
     * @param max The maximum value.
     */
    void set_range(double min, double max);
    /**
     * @brief Sets the value of the slider.
     *
     * @param value The value to set.
     */
    void set_value(double value);
    /**
     * @brief Gets the value of the slider.
     *
     * @return The value of the slider.
     */
    double get_value() const;

Q_SIGNALS:
    /**
     * @brief Signal emitted when the value of the slider changes.
     *
     * @param value The new value of the slider.
     */
    void value_changed(double);

private:
    int convert_to_int_value(double value) const;
    double m_min = 0.0f;
    double m_max = 1.0f;
};

OPENDCC_NAMESPACE_CLOSE