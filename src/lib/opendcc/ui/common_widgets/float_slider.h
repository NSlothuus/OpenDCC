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
 * @brief The FloatSlider class is a custom slider widget that supports floating-point values.
 */
class OPENDCC_UI_COMMON_WIDGETS_API FloatSlider : public QSlider
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a FloatSlider object with the specified orientation and parent widget.
     *
     * @param orientation The orientation of the slider (Qt::Horizontal or Qt::Vertical).
     * @param parent The parent widget.
     */
    FloatSlider(Qt::Orientation orientation = Qt::Horizontal, QWidget* parent = nullptr);
    ~FloatSlider() override = default;

    /**
     * @brief Sets the minimum value of the slider.
     *
     * @param min The minimum value to set.
     */
    void set_minimum(float min);
    /**
     * @brief Gets the minimum value of the slider.
     *
     * @return The minimum value of the slider.
     */
    float get_minimum() const;
    /**
     * @brief Sets the maximum value of the slider.
     *
     * @param max The maximum value to set.
     */
    void set_maximum(float max);
    /**
     * @brief Gets the maximum value of the slider.
     *
     * @return The maximum value of the slider.
     */
    float get_maximum() const;
    /**
     * @brief Sets the range of the slider.
     *
     * @param min The minimum value of the range.
     * @param max The maximum value of the range.
     */
    void set_range(float min, float max);
    /**
     * @brief Sets the value of the slider.
     *
     * @param value The value to set.
     */
    void set_value(float value);
    /**
     * @brief Gets the current value of the slider.
     *
     * @return The current value of the slider.
     */
    float get_value() const;

Q_SIGNALS:
    /**
     * @brief Signal emitted when the value of the slider changes.
     *
     * @param value The new value of the slider.
     */
    void value_changed(float);

private:
    int convert_to_int_value(float value) const;

    float m_min = 0.0f;
    float m_max = 1.0f;
    static const int s_step_size;

protected:
    virtual void resizeEvent(QResizeEvent* event) override;

    virtual void wheelEvent(QWheelEvent* e) override;
};
OPENDCC_NAMESPACE_CLOSE
