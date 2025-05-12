// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/ui/qt_utils.h"
#include <QRegularExpression>

OPENDCC_NAMESPACE_OPEN
namespace utils
{
    QString from_camel_case(const QString& s)
    {

        static QRegularExpression regExp1 { "(.)([A-Z][a-z]+)" };
        static QRegularExpression regExp2 { "([a-z0-9])([A-Z])" };
        QString result = s;
        result.replace(regExp1, "\\1_\\2");
        result.replace(regExp2, "\\1_\\2");
        return result.toLower();
    }

    void action_set_object_name_from_text(QAction* action, const QString& prefix, const QString& suffix)
    {
        QString new_name = from_camel_case(action->text());
        if (!prefix.isEmpty())
        {
            new_name.prepend(prefix + "_");
        }
        if (!suffix.isEmpty())
        {
            new_name.append("_" + suffix);
        }
        action->setObjectName(new_name);
    }

    void menu_set_object_name_from_title(QMenu* menu, const QString& prefix, const QString& suffix)
    {
        QString new_name = from_camel_case(menu->title());
        if (!prefix.isEmpty())
        {
            new_name.prepend(prefix + "_");
        }
        if (!suffix.isEmpty())
        {
            new_name.append("_" + suffix);
        }
        menu->setObjectName(new_name);
    }
}
OPENDCC_NAMESPACE_CLOSE
