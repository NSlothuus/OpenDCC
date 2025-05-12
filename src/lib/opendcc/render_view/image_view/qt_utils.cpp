// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "qt_utils.h"

#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtGui/QPainter>

OPENDCC_NAMESPACE_OPEN

DoubleSlider::DoubleSlider(Qt::Orientation orientation, QWidget* parent /* = 0 */)
    : QSlider(orientation, parent)
{
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlot(int)));
}

void DoubleSlider::valueChangedSlot(int value)
{
    double d_value = value * 0.01;
    emit valueChangedDouble(d_value);
}

void DoubleSlider::setValue(double value)
{
    QSlider::setValue(value / 0.01);
}

void DoubleSlider::setValueSilent(double value)
{
    blockSignals(true);
    QSlider::setValue(value / 0.01);
    blockSignals(false);
}

void DoubleSpinBox::setValueSilent(double value)
{
    blockSignals(true);
    QDoubleSpinBox::setValue(value);
    blockSignals(false);
}

QSize PixelInfoColorRect::sizeHint() const
{
    return QSize(15, 15);
}

void PixelInfoColorRect::paintEvent(QPaintEvent* event)
{
    const float s = 15;
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    QColor color;
    color.setRgbF(m_r, m_g, m_b, 1.0f);
    painter.setBrush(color);
    painter.translate(0, 1);
    painter.translate(s / 2, s / 2);
    painter.scale(0.7, 0.7);
    painter.translate(-s / 2, -s / 2);
    painter.drawRect(0, 0, s, s);
    painter.end();
}

QString colormode_label_text(int nchannels, RenderViewMainWindow::ColorMode mode, int channel)
{
    QString s("<span style=\"color:black\"> ");

    s += nchannels > 0 && mode == RenderViewMainWindow::ColorMode::RGB || channel == 0 && mode == RenderViewMainWindow::ColorMode::SingleChannel
             ? "<span style=\"color:red\"> R </span>"
             : "<span> R </span>";

    s += nchannels > 1 && mode == RenderViewMainWindow::ColorMode::RGB || channel == 1 && mode == RenderViewMainWindow::ColorMode::SingleChannel
             ? "<span style=\"color:green\"> G </span>"
             : "<span> G </span>";
    s += nchannels > 2 && mode == RenderViewMainWindow::ColorMode::RGB || channel == 2 && mode == RenderViewMainWindow::ColorMode::SingleChannel
             ? "<span style=\"color:blue\"> B </span>"
             : "<span> B </span>";
    s += nchannels > 3 && mode == RenderViewMainWindow::ColorMode::RGB || channel == 3 && mode == RenderViewMainWindow::ColorMode::SingleChannel
             ? "<span style=\"color:white\"> A </span>"
             : "<span> A </span>";
    s += mode == RenderViewMainWindow::ColorMode::Lumiance ? "<span style=\"color:yellow\"> L </span>" : "<span> L </span>";

    s += " </span>";
    return s;
}

OPENDCC_NAMESPACE_CLOSE
