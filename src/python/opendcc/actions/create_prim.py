# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtCore, QtGui, QtWidgets
import opendcc.core as dcc_core
from pxr import Sdf
from opendcc import cmds
from opendcc.stage_utils import apply_schema_to_spec


class CreatePrimAction(QtWidgets.QAction):
    def __init__(self, prim_name, prim_type, icon=None, label=None, parent=None):
        self._prim_name = prim_name
        self._prim_type = prim_type

        QtWidgets.QAction.__init__(
            self, "{0}".format(prim_name) if not label else label, parent=parent
        )
        if icon:
            self.setIcon(icon)
        self.triggered.connect(self.create_prim)

    def create_prim(self):
        res = cmds.create_prim(self._prim_name, self._prim_type)
        return res.get_result() if res.is_successful() else None


class CreateLightPrimAction(CreatePrimAction):

    _hooks = []

    @staticmethod
    def add_hook(hook):
        CreateLightPrimAction._hooks.append(hook)

    def create_prim(self):
        new_path = CreatePrimAction.create_prim(self)

        app = dcc_core.Application.instance()
        session = app.get_session()
        stage = session.get_current_stage()
        if not stage:
            return
        layer = stage.GetEditTarget().GetLayer()
        prim_spec = layer.GetPrimAtPath(new_path)
        if not prim_spec:
            return

        for hook in type(self)._hooks:
            hook(prim_spec)
