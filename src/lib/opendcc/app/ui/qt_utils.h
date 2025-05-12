/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QString>
#include <QAction>
#include <QMenu>

OPENDCC_NAMESPACE_OPEN
namespace utils
{
    QString from_camel_case(const QString& s);

    void action_set_object_name_from_text(QAction* action, const QString& prefix = "", const QString& suffix = "");

    void menu_set_object_name_from_title(QMenu* menu, const QString& prefix = "", const QString& suffix = "");
}
OPENDCC_NAMESPACE_CLOSE
