# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtCore, QtGui, QtWidgets
import opendcc.core as dcc_core
from pxr import Sdf
from opendcc.usd_ui_utils import remove_prims
from opendcc.undo import UsdEditsUndoBlock
from .custom_widget_action import CustomAction

from opendcc.i18n import i18n


class RemoveAction(CustomAction):
    def __init__(self, parent=None):
        CustomAction.__init__(self, i18n("main_menu.edit", "Delete"), parent=parent)
        self.custom_triggered.connect(self.remove)

        shortcut = QtGui.QKeySequence(QtCore.Qt.Key_Delete)
        self.setShortcut(shortcut, self.remove)

    def remove(self):
        app = dcc_core.Application.instance()
        selected = app.get_prim_selection()

        if len(selected) < 1:
            return

        session = app.get_session()
        stage = session.get_current_stage()
        with Sdf.ChangeBlock(), UsdEditsUndoBlock():
            res = remove_prims(selected, stage, ask=False)
