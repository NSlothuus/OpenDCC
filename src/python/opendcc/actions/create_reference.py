# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from PySide6 import QtCore, QtGui, QtWidgets
import opendcc.core as dcc_core
from pxr import Sdf, Usd
from opendcc.stage_utils import (
    find_new_name_for_prim,
    has_parent_same_child,
)
from opendcc import cmds
from opendcc.undo import UsdEditsUndoBlock

from opendcc.i18n import i18n

import os


class CreateReferenceAction(QtWidgets.QAction):
    def __init__(self, title=None, icon=None, parent=None):
        if not title:
            title = i18n("shelf.toolbar.general", "Create reference")
        QtWidgets.QAction.__init__(self, title, parent=parent)
        if icon:
            self.setIcon(icon)
        self.triggered.connect(self.do)
        self.setIcon(QtGui.QIcon(":icons/create_reference.png"))

    def do(self):
        app = dcc_core.Application.instance()
        session = app.get_session()
        stage = session.get_current_stage()
        if not stage:
            return

        layer = stage.GetRootLayer()

        # select file to reference on
        file_name, filter = QtWidgets.QFileDialog.getOpenFileName(
            self.parent(),
            "Select file",
            layer.realPath,
            filter=dcc_core.Application.get_file_extensions(),
        )
        if not file_name or not os.path.isfile(file_name):
            return

        ref_layer = Sdf.Layer.FindOrOpen(file_name)
        # if we have default prim metadata, we just get, else we get first root prim
        ref_root_prim_name = (
            ref_layer.defaultPrim
            if ref_layer.HasDefaultPrim()
            else list(ref_layer.rootPrims.keys())[0]
        )

        # find name to new prim at current stage, avoiding collisions
        root_prim = stage.GetPrimAtPath("/")
        new_path = Sdf.Path("/{0}".format(ref_root_prim_name))

        if has_parent_same_child(root_prim, ref_root_prim_name):
            new_path = find_new_name_for_prim(new_path, root_prim)

        # create new prim, set reference on selected prim and select new prim
        with UsdEditsUndoBlock():
            # create prim
            prim_spec = Sdf.PrimSpec(layer, new_path.name, Sdf.SpecifierDef)
            res = bool(prim_spec)

            if res:
                new_prim = stage.GetPrimAtPath(new_path)
                refs = new_prim.GetReferences()

                # if we have metadata we reference without prim specifier
                if ref_layer.HasDefaultPrim():
                    refs.AddReference(ref_layer.identifier)
                else:
                    refs.AddReference(ref_layer.identifier, "/%s" % ref_root_prim_name)
                cmds.select(dcc_core.SelectionList([new_path]), replace=True)
