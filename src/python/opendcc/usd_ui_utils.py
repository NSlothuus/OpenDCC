# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets
from pxr import Sdf


def reload_layers(layers, widget_parent):
    if any([layer.dirty for layer in layers]):
        buttons = QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.Cancel
        answer = QtWidgets.QMessageBox.question(
            widget_parent,
            "Confirm Reload Layer",
            "Layer you want to reload has changes. Do you still want to reload it and discard changes?",
            buttons=buttons,
            defaultButton=QtWidgets.QMessageBox.Yes,
        )
        if answer == QtWidgets.QMessageBox.Cancel:
            return
    for layer in layers:
        layer.Reload()


def remove_prims(prims_paths, stage, ask=True, messagebox_parent=None):
    buttons = QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.Cancel
    if len(prims_paths) > 1:
        buttons |= QtWidgets.QMessageBox.YesToAll
    with Sdf.ChangeBlock():
        for path in prims_paths:
            if ask:
                answer = QtWidgets.QMessageBox.question(
                    messagebox_parent,
                    "Confirm Prim Removal",
                    "Remove prim/prim edits (and any children) at {0}?".format(path),
                    buttons=buttons,
                    defaultButton=QtWidgets.QMessageBox.Yes,
                )
                if answer == QtWidgets.QMessageBox.Cancel:
                    return
                elif answer == QtWidgets.QMessageBox.YesToAll:
                    ask = False
            stage.RemovePrim(path)
