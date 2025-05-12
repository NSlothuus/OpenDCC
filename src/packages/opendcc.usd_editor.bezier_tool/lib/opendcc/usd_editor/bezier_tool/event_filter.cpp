// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/bezier_tool/event_filter.h"

#include <QKeyEvent>
#include <QShortcutEvent>

OPENDCC_NAMESPACE_OPEN

bool EventFilter::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::ShortcutOverride)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Delete)
        {
            event->accept();
            return true;
        }
    }

    return QObject::eventFilter(object, event);
}

OPENDCC_NAMESPACE_CLOSE
