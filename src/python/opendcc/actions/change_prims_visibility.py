# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtCore, QtGui, QtWidgets
import opendcc.core as dcc_core
from pxr import Sdf

from opendcc.undo import UsdEditsUndoBlock
from .custom_widget_action import CustomAction


class VisibilityAction(CustomAction):
    def __init__(self, title, shortcut, state, parent=None):
        CustomAction.__init__(self, title, parent=parent)
        self.custom_triggered.connect(self.do)
        self._state = state

        self.setShortcut(shortcut, self.do)

    def do(self):
        app = dcc_core.Application.instance()
        selected = app.get_prim_selection()
        if len(selected) < 1:
            return

        session = app.get_session()
        stage = session.get_current_stage()

        with Sdf.ChangeBlock(), UsdEditsUndoBlock():
            for path in selected:
                prim = stage.GetPrimAtPath(path)
                if prim and prim.HasAttribute("visibility"):
                    attr = prim.GetAttribute("visibility")
                    attr.Set(self._state)
