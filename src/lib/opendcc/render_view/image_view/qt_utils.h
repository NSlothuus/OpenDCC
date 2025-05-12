/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QtWidgets/QSlider>
#include <QtWidgets/QDoubleSpinBox>
#include "app.h"

OPENDCC_NAMESPACE_OPEN

class PixelInfoColorRect : public QWidget
{
    Q_OBJECT
public:
    PixelInfoColorRect()
        : QWidget()
    {
        m_r = 0.0;
        m_g = 0.0;
        m_b = 0.0;
    };
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* event) override;
    void setColor(float r, float g, float b)
    {
        m_r = r;
        m_g = g;
        m_b = b;
        repaint();
    };

private:
    float m_r, m_g, m_b;
};

class DoubleSlider : public QSlider
{
    Q_OBJECT
public:
    DoubleSlider(Qt::Orientation orientation, QWidget* parent = 0);

Q_SIGNALS:
    void valueChangedDouble(double value);
public Q_SLOTS:
    void setValue(double value);
    void setValueSilent(double value);
private Q_SLOTS:
    void valueChangedSlot(int value);
};

class DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    DoubleSpinBox()
        : QDoubleSpinBox() {};

public Q_SLOTS:
    void setValueSilent(double value);
};

QString colormode_label_text(int nchannels, RenderViewMainWindow::ColorMode mode, int channel);

OPENDCC_NAMESPACE_CLOSE
