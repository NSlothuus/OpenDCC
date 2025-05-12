/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <QWidget>
#include <QColor>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_UI_COMMON_WIDGETS_API HTinySlider : public QWidget
{
    Q_OBJECT;

    QColor m_slider_color = QColor::fromRgbF(0.14, 0.40, 0.69);

    bool m_pressed = false;

    double m_value = 0.5;
    double m_min = 0;
    double m_max = 1;

public:
    HTinySlider(QWidget* parent = 0);

    QColor slider_color() const;

    double value() const;
    double min() const;
    double max() const;

public Q_SLOTS:

    void slider_color(QColor val);
    void value(double val);
    void min(double val);
    void max(double val);

Q_SIGNALS:

    void start_changing(double old_val);
    void changing(double val);
    void end_changing(double val);
};

class OPENDCC_UI_COMMON_WIDGETS_API VTinySlider : public QWidget
{
    Q_OBJECT;

    QColor m_slider_color = QColor::fromRgbF(0.14, 0.40, 0.69);

    bool m_pressed = false;

    double m_value = 0.5;
    double m_min = 0;
    double m_max = 1;

public:
    VTinySlider(QWidget* parent = 0);

    QColor slider_color() const;

    double value() const;
    double min() const;
    double max() const;

public Q_SLOTS:

    void slider_color(QColor val);
    void value(double val);
    void min(double val);
    void max(double val);

Q_SIGNALS:

    void start_changing(double old_val);
    void changing(double val);
    void end_changing(double val);
};
OPENDCC_NAMESPACE_CLOSE
