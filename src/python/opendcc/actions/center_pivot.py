# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from PySide6 import QtWidgets
import opendcc.cmds as cmds

from opendcc.i18n import i18n


class CenterPivotAction(QtWidgets.QAction):
    def __init__(self, parent=None):
        QtWidgets.QAction.__init__(self, i18n("main_menu.modify", "Center Pivot"), parent=parent)
        self.triggered.connect(self.do)

    def do(self):
        cmds.center_pivot()
