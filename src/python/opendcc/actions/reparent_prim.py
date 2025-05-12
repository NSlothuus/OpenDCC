# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtCore, QtGui, QtWidgets
import opendcc.core as dcc_core
from opendcc import cmds
from .custom_widget_action import CustomAction
from .options_widget import OptionsWidget, make_option

from opendcc.i18n import i18n


class ReparentAction(CustomAction):
    def __init__(self, parent=None):
        CustomAction.__init__(self, i18n("edit_menu.reparent_action", "Parent"), parent)
        shortcut = QtGui.QKeySequence(QtCore.Qt.Key_P)
        self.setShortcut(shortcut, self.do)
        self.on_checkbox_clicked(self.show_options)
        self.custom_triggered.connect(self.do)

        options = []
        preserve_transform_option = make_option(
            i18n("edit_menu.reparent_action", "Preserve Transform"),
            "parent_command.preserve_transform",
            {"type": "bool"},
            True,
        )

        options.append(preserve_transform_option)

        self.options_widget = OptionsWidget(
            i18n("edit_menu.reparent_action", "Parent"),
            i18n("edit_menu.reparent_action", "Parent"),
            self.do,
            options,
            self.parent(),
        )

    def do(self):
        app = dcc_core.Application.instance()

        settings = dcc_core.Application.instance().get_settings()
        preserve_transform = settings.get_bool("parent_command.preserve_transform", True)
        selected = app.get_prim_selection()
        if len(selected) <= 1:
            return
        new_parent_path = selected[-1]
        paths = selected[:-1]
        cmds.parent_prim(new_parent_path, paths=paths, preserve_transform=preserve_transform)

    def show_options(self):
        self.options_widget.show()
