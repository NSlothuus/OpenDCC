# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtCore, QtGui, QtWidgets
import opendcc.core as dcc_core
from pxr import Sdf
from opendcc.undo import UsdEditsUndoBlock

from opendcc.i18n import i18n


class SetInstanceableAction(QtWidgets.QAction):
    def __init__(self, parent=None):
        QtWidgets.QAction.__init__(
            self, i18n("main_menu.modify", "Set Instanceable"), parent=parent
        )
        self.triggered.connect(self.do)

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
                prim.SetInstanceable(True)
