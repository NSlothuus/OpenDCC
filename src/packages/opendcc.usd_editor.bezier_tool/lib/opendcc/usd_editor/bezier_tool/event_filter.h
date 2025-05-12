/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include <QObject>

OPENDCC_NAMESPACE_OPEN

class EventFilter : public QObject
{
public:
    bool eventFilter(QObject *object, QEvent *event);
};

OPENDCC_NAMESPACE_CLOSE
