/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <QWidget>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QFrame>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief The PopupWindow class represents a
 * custom widget for displaying a popup window.
 *
 * It provides functionality for showing the popup window
 * and setting its content widget.
 */
class OPENDCC_UI_COMMON_WIDGETS_API PopupWindow : public QWidget
{
    Q_OBJECT;

protected:
    virtual bool eventFilter(QObject *, QEvent *) override;

public:
    /**
     * @brief Constructs a PopupWindow object with the specified parent widget.
     *
     * @param parent The parent widget.
     */
    PopupWindow(QWidget *parent = 0);

    /**
     * @brief Shows the popup window.
     */
    void show();
    /**
     * @brief Shows the popup window at the specified position.
     *
     * @param x The x-coordinate of the position.
     * @param y The y-coordinate of the position.
     */
    void show(int x, int y);
    /**
     * @brief Sets the content widget of the popup window.
     *
     * @param widget The content widget to set.
     */
    void set_widget(QWidget *);
};

OPENDCC_NAMESPACE_CLOSE