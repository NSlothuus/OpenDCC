/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <functional>

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief This class represents a widget for a canvas.
 */
class OPENDCC_UI_COMMON_WIDGETS_API CanvasWidget : public QWidget
{
    Q_OBJECT;

public:
    std::function<void(QPaintEvent *)> paint_event;
    std::function<void(QMouseEvent *)> mouse_press_event;
    std::function<void(QMouseEvent *)> mouse_move_event;
    std::function<void(QMouseEvent *)> mouse_release_event;
    /**
     * @brief Constructs a CanvasWidget using the specified window flag.
     *
     * @param parent The parent widget.
     * @param flags The window flags.
     */
    CanvasWidget(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags());

    /**
     * @brief Overridden paint event handler.
     *
     * @param e The paint event.
     */
    virtual void paintEvent(QPaintEvent *e) override;
    /**
     * @brief Overridden mouse press event handler.
     *
     * @param e The mouse press event.
     */
    virtual void mousePressEvent(QMouseEvent *e) override;
    /**
     * @brief Overridden mouse release event handler.
     *
     * @param e The mouse release event.
     */
    virtual void mouseReleaseEvent(QMouseEvent *e) override;
    /**
     * @brief Overridden mouse move event handler.
     *
     * @param e The mouse move event.
     */
    virtual void mouseMoveEvent(QMouseEvent *e) override;
};
OPENDCC_NAMESPACE_CLOSE