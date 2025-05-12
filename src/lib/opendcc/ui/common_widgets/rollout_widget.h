/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"

#include <QWidget>

class QLabel;

OPENDCC_NAMESPACE_OPEN

/**
 * @brief The RolloutWidget class represents a
 * widget with a collapsible/expandable section.
 *
 * It provides functionality for setting the title, expanding
 * or collapsing the section, and setting the layout.
 */
class OPENDCC_UI_COMMON_WIDGETS_API RolloutWidget : public QWidget
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a RolloutWidget object with the specified title,
     * expandable flag, and parent widget.
     *
     * @param title The title of the widget.
     * @param expandable Flag indicating whether the section is expandable.
     * @param parent The parent widget.
     */
    RolloutWidget(QString title, bool expandable = true, QWidget* parent = nullptr);
    ~RolloutWidget();

    /**
     * @brief Sets the section expanded or collapsed.
     *
     * @param expanded Flag indicating whether the section should be expanded.
     */
    void set_expanded(bool expanded);
    /**
     * @brief Sets the layout of the widget.
     *
     * @param layout The layout to set.
     */
    void set_layout(QLayout* layout);

Q_SIGNALS:
    /**
     * @brief Signal emitted when the section is clicked.
     *
     * @param expanded Flag indicating whether the section is expanded.
     */
    void clicked(bool expanded);

private:
    bool m_expandable;
    bool m_expanded = true;
    QLabel* m_arrow;

    QWidget* m_header;
    QWidget* m_content;

    bool m_header_pressed = false;

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
};

OPENDCC_NAMESPACE_CLOSE