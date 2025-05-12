/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <QWidget>

#include "opendcc/ui/common_widgets/ramp_widget.h"

OPENDCC_NAMESPACE_OPEN

/**
 * @brief The VecWidget class represents a widget for displaying
 * and editing a collection of values.
 */
class OPENDCC_UI_COMMON_WIDGETS_API VecWidget : public QWidget
{
    Q_OBJECT;

    int m_size = 1;
    FloatWidget* m_floats[4];

public:
    /**
     * @brief Constructs a VecWidget object with the specified size,
     * type, and parent widget.
     *
     * @param size The size of the collection.
     * @param as_int Flag indicating whether the collection values should be treated as integers.
     * @param slideable Flag indicating whether the collection values should be slideable.
     * @param parent The parent widget.
     */
    VecWidget(int size = 1, bool as_int = false, bool slideable = false, QWidget* parent = 0);
    /**
     * @brief Sets the value of the collection.
     *
     * @param val The value to set.
     */
    void value(float* val);
    /**
     * @brief Sets the value of the collection.
     *
     * @param val The value to set.
     */
    void value(int* val);

    /**
     * @brief Gets the value of the collection as integers.
     *
     * @param val The output array to store the values.
     */
    void get_value(int* val);
    /**
     * @brief Gets the value of the collection as floats.
     *
     * @param val The output array to store the values.
     */
    void get_value(float* val);

Q_SIGNALS:
    /**
     * @brief Signal emitted when the value of the collection changes.
     */
    void changed();
};
OPENDCC_NAMESPACE_CLOSE