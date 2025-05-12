# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtCore, QtGui
import opendcc.core as dcc_core

from opendcc.i18n import i18n


def snap_mode_button():
    snap_mode_widget = QtWidgets.QToolButton()
    snap_mode_widget.setPopupMode(QtWidgets.QToolButton.InstantPopup)

    snap_mode_widget.setToolTip(i18n("toolbars.tools", "Snap Mode"))

    snap_mode_mode_menu = QtWidgets.QMenu(snap_mode_widget)
    snap_mode_group = QtWidgets.QActionGroup(snap_mode_widget)

    snap_off_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/small_snap_off"), i18n("toolbars.tools", "Off")
    )
    snap_off_action.setData(0)
    snap_off_action.setCheckable(True)
    snap_mode_mode_menu.addAction(snap_off_action)
    snap_mode_group.addAction(snap_off_action)

    snap_mode_mode_menu.addSeparator()

    relative_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/small_relative"), i18n("toolbars.tools", "Relative")
    )
    relative_action.setData(1)
    relative_action.setCheckable(True)
    snap_mode_mode_menu.addAction(relative_action)
    snap_mode_group.addAction(relative_action)

    absolute_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/small_absolute"), i18n("toolbars.tools", "Absolute")
    )
    absolute_action.setData(2)
    absolute_action.setCheckable(True)
    snap_mode_mode_menu.addAction(absolute_action)
    snap_mode_group.addAction(absolute_action)

    snap_mode_mode_menu.addSeparator()

    grid_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/small_snap_grid"), i18n("toolbars.tools", "Grid")
    )
    grid_action.setData(3)
    grid_action.setCheckable(True)
    snap_mode_mode_menu.addAction(grid_action)
    snap_mode_group.addAction(grid_action)

    vertex_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/small_snap_vertex"), i18n("toolbars.tools", "Vertex")
    )
    vertex_action.setData(4)
    vertex_action.setCheckable(True)
    snap_mode_mode_menu.addAction(vertex_action)
    snap_mode_group.addAction(vertex_action)

    edge_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/small_snap_edge"), i18n("toolbars.tools", "Edge")
    )
    edge_action.setData(5)
    edge_action.setCheckable(True)
    snap_mode_mode_menu.addAction(edge_action)
    snap_mode_group.addAction(edge_action)

    edge_center_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/small_snap_edge_center"), i18n("toolbars.tools", "Edge Center")
    )
    edge_center_action.setData(6)
    edge_center_action.setCheckable(True)
    snap_mode_mode_menu.addAction(edge_center_action)
    snap_mode_group.addAction(edge_center_action)

    face_center_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/small_snap_face_center"), i18n("toolbars.tools", "Face Center")
    )
    face_center_action.setData(7)
    face_center_action.setCheckable(True)
    snap_mode_mode_menu.addAction(face_center_action)
    snap_mode_group.addAction(face_center_action)

    object_surface_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/small_snap_object_surface"), i18n("toolbars.tools", "Object Surface")
    )
    object_surface_action.setData(8)
    object_surface_action.setCheckable(True)
    snap_mode_mode_menu.addAction(object_surface_action)
    snap_mode_group.addAction(object_surface_action)

    snap_mode_widget.setMenu(snap_mode_mode_menu)

    app = dcc_core.Application.instance()
    settings = app.get_settings()
    setting_name = "viewport.move_tool.snap_mode"

    tooltip = {
        0: i18n("toolbars.tools", "Snap Mode (<b>Off</b>)"),
        1: i18n("toolbars.tools", "Snap Mode (<b>Relative</b>)"),
        2: i18n("toolbars.tools", "Snap Mode (<b>Absolute</b>)"),
        3: i18n("toolbars.tools", "Snap Mode (<b>Grid</b>)"),
        4: i18n("toolbars.tools", "Snap Mode (<b>Vertex</b>)"),
        5: i18n("toolbars.tools", "Snap Mode (<b>Edge</b>)"),
        6: i18n("toolbars.tools", "Snap Mode (<b>Edge Center</b>)"),
        7: i18n("toolbars.tools", "Snap Mode (<b>Face Center</b>)"),
        8: i18n("toolbars.tools", "Snap Mode (<b>Object Surface</b>)"),
    }

    def setting_changed(name, mode, type):
        snap_mode_group.blockSignals(True)
        for action in snap_mode_group.actions():
            if action.data() == mode:
                action.setChecked(True)
                snap_mode_widget.setIcon(action.icon())
                snap_mode_widget.setToolTip(tooltip[mode])
                break
        snap_mode_group.blockSignals(False)

    mode = settings.get_int(setting_name, 0)
    setting_changed(setting_name, mode, None)
    cb_id = settings.register_setting_changed(setting_name, setting_changed)

    def mode_changed(action):
        mode = action.data()
        settings.set_int(setting_name, mode)

    snap_mode_group.triggered.connect(mode_changed)

    return snap_mode_widget


def init_snap_mode_toolbar():
    main_win = dcc_core.Application.instance().get_main_window()

    snap_mode_toolbar = QtWidgets.QToolBar(i18n("toolbars.tools", "Snap Mode"))
    snap_mode_toolbar.setObjectName("snap_mode_toolbar")
    snap_mode_toolbar.setIconSize(QtCore.QSize(20, 20))

    snap_mode_toolbar.addWidget(snap_mode_button())

    main_win.addToolBar(QtCore.Qt.TopToolBarArea, snap_mode_toolbar)
