/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <QObject>
#include <QEvent>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API GlobalEventFilter : public QObject
{
    Q_OBJECT
public:
    GlobalEventFilter(QObject* parent = nullptr);
    ~GlobalEventFilter() {}

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

private:
    bool check_type(QObject* object) const;
    QObject* get_panel(QObject* object) const;
    bool send(QObject* object, QEvent* event) const;
    void set_focus(QObject* object) const;
};
OPENDCC_NAMESPACE_CLOSE
