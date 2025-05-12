# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from opendcc.packaging import PackageEntryPoint
import opendcc.core as dcc_core
from Qt import QtWidgets, QtGui

from opendcc.i18n import i18n


class BulletPhysicsEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        app = dcc_core.Application.instance()
        app.register_event_callback(app.EventType.AFTER_UI_LOAD, lambda: self.init_ui())

    def add_bullet_toolbar(self):
        import opendcc.usd_editor.bullet_physics
        from . import bullet_physics_control as bullet_physics_control
        from opendcc.actions.tab_toolbar import Shelf

        tool_bar_bullet_name = i18n("shelf.toolbar", "Rigid Body")
        tool_bar_bullet = QtWidgets.QToolBar(tool_bar_bullet_name)

        bullet_on = tool_bar_bullet.addAction(
            QtGui.QIcon(":/icons/bullet_on"),
            i18n("shelf.toolbar.rigid_body", "Turn On/Off Bullet Engine"),
        )
        bullet_on.setCheckable(True)

        bullet_play = tool_bar_bullet.addAction(
            QtGui.QIcon(":/icons/bullet_play"),
            i18n("shelf.toolbar.rigid_body", "Start/Stop Simulation"),
        )
        bullet_play.setCheckable(True)

        tool_bar_bullet.addSeparator()

        bullet_dynamic = QtWidgets.QToolButton()
        bullet_dynamic.setIcon(QtGui.QIcon(":/icons/bullet_dynamic"))
        bullet_dynamic.setToolTip(i18n("shelf.toolbar.rigid_body", "Make Selected Objects Dynamic"))
        bullet_dynamic.setPopupMode(QtWidgets.QToolButton.MenuButtonPopup)
        tool_bar_bullet.addWidget(bullet_dynamic)

        bullet_static = tool_bar_bullet.addAction(
            QtGui.QIcon(":/icons/bullet_static"),
            i18n("shelf.toolbar.rigid_body", "Make Selected Objects Static"),
        )

        tool_bar_bullet.addSeparator()

        bullet_relax = tool_bar_bullet.addAction(
            QtGui.QIcon(":/icons/bullet_relax"), i18n("shelf.toolbar.rigid_body", "Relax")
        )

        bullet_clear_selected = tool_bar_bullet.addAction(
            QtGui.QIcon(":/icons/clear_selected"),
            i18n("shelf.toolbar.rigid_body", "Clear Selected"),
        )

        bullet_clear_all = tool_bar_bullet.addAction(
            QtGui.QIcon(":/icons/reset_output"), i18n("shelf.toolbar.rigid_body", "Clear All")
        )

        bullet_debug = tool_bar_bullet.addAction(
            QtGui.QIcon(":/icons/bug"), i18n("shelf.toolbar.rigid_body", "Debug View")
        )
        bullet_debug.setCheckable(True)

        mesh_approximation = QtWidgets.QMenu()
        mesh_approximation_group = QtWidgets.QActionGroup(mesh_approximation)

        CONVEX_HULL = QtWidgets.QAction(i18n("shelf.toolbar.rigid_body", "CONVEX_HULL"))
        CONVEX_HULL.setCheckable(True)
        CONVEX_HULL.setChecked(True)
        CONVEX_DECOMPOSITION = QtWidgets.QAction(
            i18n("shelf.toolbar.rigid_body", "CONVEX_DECOMPOSITION")
        )
        CONVEX_DECOMPOSITION.setCheckable(True)
        BOX = QtWidgets.QAction(i18n("shelf.toolbar.rigid_body", "BOX"))
        BOX.setCheckable(True)

        mesh_approximation.addAction(CONVEX_HULL)
        mesh_approximation_group.addAction(CONVEX_HULL)
        mesh_approximation.addAction(CONVEX_DECOMPOSITION)
        mesh_approximation_group.addAction(CONVEX_DECOMPOSITION)
        mesh_approximation.addAction(BOX)
        mesh_approximation_group.addAction(BOX)

        bullet_dynamic.setMenu(mesh_approximation)

        bullet_buttons = [
            bullet_play,
            bullet_dynamic,
            bullet_static,
            bullet_relax,
            bullet_clear_selected,
            bullet_clear_all,
            bullet_debug,
            mesh_approximation,
        ]
        for button in bullet_buttons:
            button.setEnabled(False)

        self.tool_bar_bullet_instance = bullet_physics_control.BulletPhysicsControlInstance(
            bullet_on, bullet_play, mesh_approximation_group, bullet_buttons
        )
        bullet_on.toggled.connect(self.tool_bar_bullet_instance.activate)
        bullet_play.toggled.connect(self.tool_bar_bullet_instance.start_simulation)
        bullet_dynamic.pressed.connect(self.tool_bar_bullet_instance.add_dyn)
        bullet_static.triggered.connect(self.tool_bar_bullet_instance.add_static)
        bullet_relax.triggered.connect(self.tool_bar_bullet_instance.relax)
        bullet_clear_selected.triggered.connect(self.tool_bar_bullet_instance.remove_bodies)
        bullet_clear_all.triggered.connect(self.tool_bar_bullet_instance.remove_all_bodies)
        bullet_debug.toggled.connect(self.tool_bar_bullet_instance.debug_draw)

        Shelf.instance().add_tab(tool_bar_bullet, tool_bar_bullet_name)

    def init_ui(self):
        self.add_bullet_toolbar()
