// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/node_editor/tab_search.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QDebug>
#include <QApplication>

OPENDCC_NAMESPACE_OPEN

TabSearchMenu::TabSearchMenu(QWidget* parent)
    : QMenu(parent)
{
    m_completer_widget = new CompleterWidget(this);

    addAction(m_completer_widget);

    setStyleSheet("QMenu { menu-scrollable: 1;}");
}

void TabSearchMenu::clear_actions()
{
    // Delete all stored QActions except completer
    // Make sure to also delete QMenu owned by TabSearchMenu,
    // created via addMenu(QString)
    auto actions = this->actions();
    for (auto action : actions)
    {
        if (action == m_completer_widget)
            continue;
        removeAction(action);
        if (auto menu = action->menu())
            menu->deleteLater();
        else
            action->deleteLater();
    }
}

bool TabSearchMenu::event(QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        const auto key = key_event->key();
        if (key == Qt::Key_Up || key == Qt::Key_Down)
        {
            auto menu = m_completer_widget->get_menu();
            if (!menu->isHidden())
            {
                return true;
            }
        }
    }
    else if (event->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);

        auto menu = m_completer_widget->get_menu();

        QAction* selected_action = menu->isHidden() ? nullptr : menu->activeAction();
        const auto key = key_event->key();

        if (key == Qt::Key_Escape)
        {
            close();
        }
        else if (key == Qt::Key_Tab)
        {
            if (selected_action)
            {
                selected_action->activate(QAction::Trigger);
                close();
            }
            else
            {
                close();
            }
        }
        else if (key == Qt::Key_Backspace)
        {
            if (!m_current_text.isEmpty())
            {
                m_current_text = m_current_text.left(m_current_text.length() - 1);
            }
            update_completer();
        }
        else if (key == Qt::Key_Select || key == Qt::Key_Down || key == Qt::Key_Up || key == Qt::Key_Back || key == Qt::Key_Tab ||
                 key == Qt::Key_Right || key == Qt::Key_Left)
        {
            if (!selected_action)
            {
                if (activeAction())
                {
                    if (activeAction()->menu())
                    {
                        if (!activeAction()->menu()->isHidden())
                        {
                            releaseKeyboard();
                        }
                        else
                        {
                            grabKeyboard();
                        }
                    }
                }

                return QMenu::event(event);
            }
            else
            {
                QKeyEvent* keypress = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier, key_event->text());
                qApp->postEvent(menu, keypress);
                return true;
            }
        }
        else if (key == Qt::Key_Enter || key == Qt::Key_Return)
        {
            if (selected_action)
            {
                selected_action->activate(QAction::Trigger);
                close();
            }
        }
        else
        {
            m_current_text += key_event->text();
            update_completer();
        }
        return true;
    }
    return QMenu::event(event);
}

void TabSearchMenu::closeEvent(QCloseEvent* event)
{
    releaseKeyboard();
    m_current_text.clear();
    m_completer_widget->set_input_text(m_current_text);
    m_completer_widget->get_menu()->close();
    QMenu::closeEvent(event);
}

void TabSearchMenu::showEvent(QShowEvent* event)
{
    grabKeyboard();
}

void TabSearchMenu::update_completer()
{
    QList<QAction*> all_actions;
    for (auto action : actions())
    {
        auto menu = action->menu();
        if (menu)
        {
            collect_actions(all_actions, menu);
        }
        else if (!action->text().isEmpty())
        {
            all_actions.append(action);
        }
    }

    m_completer_widget->build_menu(m_current_text, all_actions, this);
    update_completer_text();
}

void TabSearchMenu::collect_actions(QList<QAction*>& all_actions, QMenu* menu)
{
    for (auto action : menu->actions())
    {
        auto menu = action->menu();
        if (menu)
        {
            collect_actions(all_actions, menu);
        }
        else if (!action->text().isEmpty())
        {
            all_actions.append(action);
        }
    }
}

void TabSearchMenu::update_completer_text()
{
    m_completer_widget->set_input_text(m_current_text);
}

CompleterWidget::CompleterWidget(TabSearchMenu* parent)
    : QWidgetAction(parent)
{
    m_search_input = new QWidget;

    m_completing_menu = new CompletingItemsMenu(parent);

    auto layout = new QHBoxLayout;
    m_search_input->setLayout(layout);

    m_search_icon = new QLabel;
    m_search_icon->setScaledContents(true);
    m_search_icon->setFixedSize(QSize(16, 16));
    m_search_icon->setPixmap(QPixmap(":/icons/small_search"));

    m_search_text = new QLabel(m_default_text);

    layout->addWidget(m_search_icon);
    layout->addWidget(m_search_text);
    layout->addStretch();

    setDefaultWidget(m_search_input);
}

void CompleterWidget::set_input_text(const QString& value)
{
    if (!value.isEmpty())
    {
        QAction* active_action = m_completing_menu->activeAction();
        if (active_action)
        {
            QString active_text = active_action->text();
            QString result = "<b>" + value + "</b>" + active_text.right(active_text.length() - value.length());
            m_search_text->setText(result);
        }
        else
        {
            m_search_text->setText(value);
        }
    }
    else
    {
        m_search_text->setText(m_default_text);
    }
}

void CompleterWidget::build_menu(const QString& value, const QList<QAction*>& actions, QMenu* parent_menu)
{
    m_completing_menu->clear();

    if (!value.isEmpty())
    {
        QList<QAction*> filtered_actions;

        const QString lower_value = value.toLower();

        for (auto action : actions)
        {
            if (action->text().toLower().startsWith(lower_value))
                filtered_actions.append(action);
        }

        if (!filtered_actions.isEmpty())
        {
            for (auto action : filtered_actions)
            {
                m_completing_menu->addAction(action);
            }
            m_completing_menu->setActiveAction(filtered_actions[0]);
            auto position = m_search_input->mapToGlobal(QPoint(m_search_input->width(), 0));
            m_completing_menu->popup(position);
        }
        else
        {
            m_completing_menu->close();
        }
    }
    else
    {
        m_completing_menu->close();
    }
}

CompletingItemsMenu::CompletingItemsMenu(TabSearchMenu* parent_menu)
    : QMenu(parent_menu)
{
    m_parent_menu = parent_menu;
    connect(this, &QMenu::triggered, this, &CompletingItemsMenu::on_triggered);
    connect(this, &QMenu::hovered, this, &CompletingItemsMenu::on_hovered);
}

void CompletingItemsMenu::on_triggered(QAction* action)
{
    m_parent_menu->close();
}

void CompletingItemsMenu::on_hovered(QAction* action)
{
    m_parent_menu->update_completer_text();
}

void CompletingItemsMenu::keyPressEvent(QKeyEvent* event)
{
    const auto key = event->key();
    if (key == Qt::Key_Select || key == Qt::Key_Down || key == Qt::Key_Up || key == Qt::Key_Back || key == Qt::Key_Return || key == Qt::Key_Enter)
    {
        QMenu::keyPressEvent(event);
    }
    else
    {
        QApplication::sendEvent(m_parent_menu, event);
    }
}
OPENDCC_NAMESPACE_CLOSE
