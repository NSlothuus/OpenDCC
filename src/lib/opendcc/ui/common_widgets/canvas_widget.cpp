// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <QBoxLayout>
#include <QPushButton>

#include "opendcc/ui/common_widgets/canvas_widget.h"

OPENDCC_NAMESPACE_OPEN

void CanvasWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (mouse_move_event)
        mouse_move_event(e);
    QWidget::mouseMoveEvent(e);
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (mouse_release_event)
        mouse_release_event(e);
    QWidget::mouseReleaseEvent(e);
}

void CanvasWidget::mousePressEvent(QMouseEvent *e)
{
    if (mouse_press_event)
        mouse_press_event(e);
    QWidget::mousePressEvent(e);
}

void CanvasWidget::paintEvent(QPaintEvent *e)
{
    if (paint_event)
        paint_event(e);
}

CanvasWidget::CanvasWidget(QWidget *parent /*= 0*/, Qt::WindowFlags flags /*= 0*/)
    : QWidget(parent, flags)
{
}
OPENDCC_NAMESPACE_CLOSE
