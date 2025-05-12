# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from opendcc.packaging import PackageEntryPoint
import opendcc.core as dcc_core
from opendcc.i18n import i18n


class PointInstancerToolEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        app = dcc_core.Application.instance()
        app.register_event_callback(app.EventType.AFTER_UI_LOAD, lambda: self.init_ui())

    def init_ui(self):
        from Qt import QtWidgets, QtGui
        from opendcc.actions.tab_toolbar import Shelf
        from . import utils

        tool_bar_point_instancer_name = i18n("shelf.toolbar", "Instancer")
        tool_bar_point_instancer = QtWidgets.QToolBar(tool_bar_point_instancer_name)

        tool_bar_point_instancer.addAction(
            QtGui.QIcon(":/icons/pointinstancer"),
            i18n("shelf.toolbar.instancer", "Create Point Instancer"),
            utils.create_point_instancer,
        )
        tool_bar_point_instancer.addAction(
            QtGui.QIcon(":/icons/paint_instance"),
            i18n("shelf.toolbar.instancer", "Tool"),
            utils.create_point_instancer_context,
        )

        Shelf.instance().add_tab(tool_bar_point_instancer, tool_bar_point_instancer_name)
