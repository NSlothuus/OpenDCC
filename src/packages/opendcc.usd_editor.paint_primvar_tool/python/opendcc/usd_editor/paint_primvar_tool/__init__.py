# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from opendcc.packaging import PackageEntryPoint
import opendcc.core as dcc_core
from opendcc.i18n import i18n


class PaintPrimvarEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        app = dcc_core.Application.instance()
        app.register_event_callback(app.EventType.AFTER_UI_LOAD, lambda: self.init_ui())

    def init_ui(self):
        from Qt import QtWidgets, QtGui
        from opendcc.actions.tab_toolbar import Shelf
        from opendcc.actions.tool_context_actions import ToolContextActions, set_tool_context
        from opendcc.actions.tool_settings_button import ToolSettingsButton

        geometry_toolbar = Shelf.instance().get_tab_widget("Geometry")
        paint_primvar_action = QtWidgets.QAction(
            QtGui.QIcon(":/icons/paint"), i18n("shelf.toolbar.geometry", "Paint Primvar")
        )
        paint_primvar_action.setCheckable(True)
        ToolContextActions().get_action_group("USD").addAction(paint_primvar_action)
        paint_primvar_action.triggered.connect(lambda: set_tool_context("PaintPrimvar"))

        geometry_toolbar.addWidget(ToolSettingsButton(paint_primvar_action))
