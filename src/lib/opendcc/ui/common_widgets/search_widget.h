/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <QLineEdit>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief The SearchWidget class represents a widget for searching text.
 *
 * It inherits from QLineEdit and provides functionality
 * for creating a search input field.
 */
class OPENDCC_UI_COMMON_WIDGETS_API SearchWidget : public QLineEdit
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a SearchWidget object with the specified parent widget.
     *
     * @param parent The parent widget.
     */
    SearchWidget(QWidget* parent = nullptr);
    ~SearchWidget();

private Q_SLOTS:
    void handle_textChanged();
    void handle_action();

private:
    bool m_empty = true;
    QAction* m_action;
};

OPENDCC_NAMESPACE_CLOSE
