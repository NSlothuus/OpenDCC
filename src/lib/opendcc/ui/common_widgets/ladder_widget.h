/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"

#include <QLineEdit>
#include <QBoxLayout>
#include <QLabel>

OPENDCC_NAMESPACE_OPEN

#define LADDER_SENS 10

/**
 * @brief Class for the numeric values supporting creating a ladder.
 *
 */
class OPENDCC_UI_COMMON_WIDGETS_API ILadderable
{
public:
    /**
     * @brief Checks if the value is an integer.
     *
     * @return True if the value is an integer, false otherwise.
     */
    virtual bool is_integer() const { return false; };
    /**
     * @brief Gets the value.
     *
     * @return The value.
     */
    virtual float get_value() const { return 0; };
    /**
     * @brief Sets the value.
     *
     * @param value The value to set.
     */
    virtual void set_value(float value) {};
    /**
     * @brief Starts the process of changing the ladder.
     */
    virtual void start_changing_ladder() {};
    /**
     * @brief Stops the process of changing the ladder.
     */
    virtual void stop_changing_ladder() {};
};

/**
 * @brief The LadderScaleItem class represents an item in a ladder scale.
 *
 * It inherits from QLabel and provides functionality to set the active state, scale, and target value of the item.
 */
class OPENDCC_UI_COMMON_WIDGETS_API LadderScaleItem : public QLabel
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a LadderScaleItem with the specified parent widget.
     *
     * @param parent The parent widget.
     */
    LadderScaleItem(QWidget* parent);

    /**
     * @brief Sets the active state of the item.
     *
     * @param is_active The active state to set.
     */
    void set_active(bool is_active);
    /**
     * @brief Sets the scale of the item.
     *
     * @param i The scale value to set.
     */
    void set_scale(float i);
    /**
     * @brief Sets the target value of the item.
     *
     * @param val The target value to set.
     */
    void set_target_value(float val);
    /**
     * @brief Gets the scale of the item.
     *
     * @return The scale of the item.
     */
    float scale() const;

private:
    float m_scale = 0;
    bool m_is_active = false;
};

/**
 * @brief The LadderScale class represents a ladder scale widget providing
 * the functionality of setting the target value and getting the scale of the ladder.
 */
class OPENDCC_UI_COMMON_WIDGETS_API LadderScale : public QFrame
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a LadderScale object with the specified parent widget and scale parameters.
     *
     * @param parent The parent widget.
     * @param min The minimum value of the scale.
     * @param max The maximum value of the scale.
     * @param step_size The step size between scale values.
     */
    LadderScale(QWidget* parent = nullptr, float min = 0.0001, float max = 1000, float step_size = 10);

    /**
     * @brief Checks if the pointer position has changed.
     *
     * @param pos The new position of the pointer.
     * @return true if the pointer position has changed, false otherwise.
     */
    bool pointer_changed(QPoint pos);
    /**
     * @brief Sets the target value of the ladder scale.
     *
     * @param val The target value to set.
     */
    void set_target_value(float val);
    /**
     * @brief Gets the scale of the ladder.
     *
     * @return The scale of the ladder.
     */
    float get_scale();

private:
    float m_scale = 1;
    LadderScaleItem* m_active_scale = nullptr;
    std::vector<LadderScaleItem*> m_scale_items;
};

/**
 * @brief The LadderNumberWidget class represents a widget for displaying
 * and editing a numerical value.
 */
class OPENDCC_UI_COMMON_WIDGETS_API LadderNumberWidget : public QLineEdit
{
    Q_OBJECT;

    float value();
    void set_value(float value);

public:
    /**
     * @brief Constructs a LadderNumberWidget object with the specified
     * parent widget and configuration.
     *
     * @param parent The parent widget.
     * @param as_int Whether the widget should display integer values only.
     */
    LadderNumberWidget(QWidget* parent = nullptr, bool as_int = false);
    ~LadderNumberWidget();
    /**
     * @brief Sets the clamp range for the widget.
     *
     * @param min The minimum value of the clamp range.
     * @param max The maximum value of the clamp range.
     */
    void set_clamp(float min, float max);
    /**
     * @brief Sets the clamp minimum for the widget.
     *
     * @param max The maximum value of the clamp range.
     */
    void set_clamp_minimum(float min);
    /**
     * @brief Sets the clamp maximum for the widget.
     *
     * @param max The maximum value of the clamp range.
     */
    void set_clamp_maximum(float max);
    /**
     * @brief Clamps the current value of the widget to the clamp range.
     *
     * @return The clamped value as a QPointF object.
     */
    QPointF clamp();
    /**
     * @brief Enables or disables clamping for the widget.
     *
     * @param val true to enable clamping, false to disable.
     */
    void enable_clamp(bool val);
    /**
     * @brief Checks if the widget is currently clamped.
     *
     * @return true if the widget is clamped, false otherwise.
     */
    bool is_clamped() const;
    /**
     * @brief Enables or disables marking for the widget.
     *
     * @param val true to enable marking, false to disable.
     */
    void enable_marker(bool val);
    /**
     * @brief Sets the color of the marker for the widget in the RGB color space.
     *
     * @param r The red component of the marker color.
     * @param g The green component of the marker color.
     * @param b The blue component of the marker color.
     */
    void set_marker_color(float r, float g, float b);

protected:
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

Q_SIGNALS:
    /**
     * @brief Signal emitted when the user starts changing the value of the widget.
     */
    void start_changing();
    /**
     * @brief Signal emitted when the user is changing the value of the widget.
     */
    void changing();
    /**
     * @brief Signal emitted when the user stops changing the value of the widget.
     */
    void stop_changing();
    /**
     * @brief Signal emitted when the value of the widget changes.
     */
    void changed();

private:
    QPoint m_pos;
    QPointF m_clamp;
    LadderScale* m_ladder = nullptr;

    bool m_as_int = false;
    bool m_activated = false;
    float m_start_value = 0;
    bool m_enable_clamp = false;

    float m_r = 0;
    float m_g = 0;
    float m_b = 0;
    bool m_enable_marker = false;
};

OPENDCC_NAMESPACE_CLOSE
