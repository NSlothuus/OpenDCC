# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from opendcc.packaging import PackageEntryPoint
import opendcc.core as dcc_core

import os


class AnimEngineEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        app = dcc_core.Application.instance()
        app.register_event_callback(app.EventType.AFTER_UI_LOAD, lambda: self.init_ui())

    def init_ui(self):
        app = dcc_core.Application.instance()
        mw = app.get_main_window()
        from Qt import QtWidgets, QtCore
        from opendcc.anim_engine import core as anim_engine_core

        create_animation_on_selected_prims_action = QtWidgets.QAction("SetKey", mw)
        create_animation_on_selected_prims_action.setShortcut("S")
        create_animation_on_selected_prims_action.setShortcutContext(QtCore.Qt.ApplicationShortcut)
        create_animation_on_selected_prims_action.triggered.connect(
            lambda: anim_engine_core.session()
            .current_engine()
            .create_animation_on_selected_prims(anim_engine_core.AttributesScope.ALL)
        )
        mw.addAction(create_animation_on_selected_prims_action)

        create_animation_on_translate_on_selected_prims_action = QtWidgets.QAction(
            "SetKeyOnTranslate", mw
        )
        create_animation_on_translate_on_selected_prims_action.setShortcut("Shift+W")
        create_animation_on_translate_on_selected_prims_action.setShortcutContext(
            QtCore.Qt.ApplicationShortcut
        )
        create_animation_on_translate_on_selected_prims_action.triggered.connect(
            lambda: anim_engine_core.session()
            .current_engine()
            .create_animation_on_selected_prims(anim_engine_core.AttributesScope.TRANSLATE)
        )
        mw.addAction(create_animation_on_translate_on_selected_prims_action)

        create_animation_on_rotate_on_selected_prims_action = QtWidgets.QAction(
            "SetKeyOnRotate", mw
        )
        create_animation_on_rotate_on_selected_prims_action.setShortcut("Shift+E")
        create_animation_on_rotate_on_selected_prims_action.setShortcutContext(
            QtCore.Qt.ApplicationShortcut
        )
        create_animation_on_rotate_on_selected_prims_action.triggered.connect(
            lambda: anim_engine_core.session()
            .current_engine()
            .create_animation_on_selected_prims(anim_engine_core.AttributesScope.ROTATE)
        )
        mw.addAction(create_animation_on_rotate_on_selected_prims_action)

        create_animation_on_scale_on_selected_prims_action = QtWidgets.QAction("SetKeyOnScale", mw)
        create_animation_on_scale_on_selected_prims_action.setShortcut("Shift+R")
        create_animation_on_scale_on_selected_prims_action.setShortcutContext(
            QtCore.Qt.ApplicationShortcut
        )
        create_animation_on_scale_on_selected_prims_action.triggered.connect(
            lambda: anim_engine_core.session()
            .current_engine()
            .create_animation_on_selected_prims(anim_engine_core.AttributesScope.SCALE)
        )
        mw.addAction(create_animation_on_scale_on_selected_prims_action)
