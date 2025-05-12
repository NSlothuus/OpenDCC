// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/common_widgets/search_widget.h"
#include <QIcon>
#include <QAction>
#include <QMouseEvent>
#include <QDebug>

OPENDCC_NAMESPACE_OPEN

SearchWidget::SearchWidget(QWidget* parent /*= nullptr*/)
    : QLineEdit(parent)
{
    m_action = addAction(QIcon(":icons/small_search"), QLineEdit::LeadingPosition);

    connect(this, &QLineEdit::textChanged, this, &SearchWidget::handle_textChanged);
    connect(m_action, &QAction::triggered, this, &SearchWidget::handle_action);
}

SearchWidget::~SearchWidget() {}

void SearchWidget::handle_textChanged()
{
    if (text().length())
    {
        m_empty = false;
        m_action->setIcon(QIcon(":icons/close_dock"));
    }
    else
    {
        m_empty = true;
        m_action->setIcon(QIcon(":icons/small_search"));
    }
}

void SearchWidget::handle_action()
{
    if (!m_empty)
    {
        clear();
    }
}

OPENDCC_NAMESPACE_CLOSE
