// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/ui/global_event_filter.h"

#include <QKeyEvent>
#include <QDebug>

#include <DockWidget.h>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QAbstractSlider>
#include <QAbstractSpinBox>

#include <QApplication>
#include <QCursor>

#include <QMetaObject>
#include <QMetaMethod>

OPENDCC_NAMESPACE_OPEN

GlobalEventFilter::GlobalEventFilter(QObject* parent)
    : QObject(parent)
{
}

bool GlobalEventFilter::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        // check if the event sent to a panel
        QObject* panel = get_panel(object);

        if (panel != nullptr)
        {
            if (check_type(object))
            {
                // check the panel under mouse cursor
                QWidget* over_widget = QApplication::widgetAt(QCursor::pos());

                if (over_widget != nullptr)
                {
                    QObject* over_object = qobject_cast<QObject*>(over_widget);
                    QObject* over_panel = get_panel(over_object);

                    if (over_panel != nullptr)
                    {
                        if (panel != over_panel)
                        {
                            // QKeyEvent *ke = static_cast<QKeyEvent *>(event);
                            if (send(over_object, event))
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }

    return false;
}

bool GlobalEventFilter::check_type(QObject* object) const
{
    // these objects must take the event no matter what
    // there are probably more of these and this list needs to be expanded
    return (!object->inherits(QLineEdit::staticMetaObject.className()) && !object->inherits(QComboBox::staticMetaObject.className()) &&
            !object->inherits(QTextEdit::staticMetaObject.className()) && !object->inherits(QPlainTextEdit::staticMetaObject.className()) &&
            !object->inherits(QAbstractSlider::staticMetaObject.className()) &&
            !object->inherits(QAbstractSpinBox::staticMetaObject.className())); // I'm terribly sorry about that T_T
}

QObject* GlobalEventFilter::get_panel(QObject* object) const
{
    QObject* parent = object;
    while (parent != nullptr)
    {
        if (parent->inherits(ads::CDockWidget::staticMetaObject.className()))
        {
            return parent;
        }
        parent = parent->parent();
    }

    return nullptr;
}

bool GlobalEventFilter::send(QObject* object, QEvent* event) const
{
    QObject* parent = object;
    while (parent != nullptr)
    {
        if (parent->inherits(ads::CDockWidget::staticMetaObject.className()))
        {
            return false;
        }

        bool found = false;

        // check key pressed
        QVariant key_pressed = parent->property("unfocusedKeyEvent_enable");
        if (key_pressed.type() == QVariant::Bool)
        {
            if (key_pressed.toBool() == true)
            {
                found = true;

                // check focus
                QVariant focus = parent->property("unfocusedKeyEvent_change_focus");
                if (focus.type() == QVariant::Bool)
                {
                    if (focus.toBool() == true)
                    {
                        set_focus(parent);
                    }
                }
                else // default
                {
                    set_focus(parent);
                }

                QApplication::sendEvent(parent, event);
            }
        }

        if (found)
        {
            // check ignore
            QVariant ignore = parent->property("unfocusedKeyEvent_block_focused_keyEvent");
            if (ignore.type() == QVariant::Bool)
            {
                if (ignore.toBool() == true)
                {
                    return false;
                }
            }

            return true;
        }

        parent = parent->parent();
    }

    return false;
}

void GlobalEventFilter::set_focus(QObject* object) const
{
    QWidget* widget = qobject_cast<QWidget*>(object);
    widget->setFocus();
}
OPENDCC_NAMESPACE_CLOSE
