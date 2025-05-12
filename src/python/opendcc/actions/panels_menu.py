# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.core as dcc_core
from Qt import QtGui

import six


def fill_panels_menu(menu):
    ordered_menu = [
        (key, obj)
        for key, obj in six.iteritems(dcc_core.PanelFactory.instance().get_registered_panels())
    ]
    ordered_menu.sort(key=lambda x: x[1]["label"])
    ordered_menu.sort(key=lambda x: x[1]["group"])

    previous_group = ""

    for key, obj in ordered_menu:
        icon = obj["icon"]
        group = obj["group"]

        if previous_group != group:
            previous_group = group
            menu.addSeparator()

        if icon:
            menu.addAction(
                QtGui.QIcon(icon), obj["label"], lambda key=key: dcc_core.create_panel(key)
            )
        else:
            menu.addAction(obj["label"], lambda key=key: dcc_core.create_panel(key))
