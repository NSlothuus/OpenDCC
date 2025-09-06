# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from PySide6 import QtWidgets, QtCore, QtGui
import opendcc.core as dcc_core
from pxr import Kind

from opendcc.i18n import i18n


class ToggleLatestComponentModeAction(QtWidgets.QAction):

    COMPONENT_MODES = frozenset(
        [
            dcc_core.Application.SelectionMode.POINTS,
            dcc_core.Application.SelectionMode.EDGES,
            dcc_core.Application.SelectionMode.FACES,
            dcc_core.Application.SelectionMode.INSTANCES,
        ]
    )

    def __init__(self, parent=None):
        QtWidgets.QAction.__init__(self, "Toggle Latest ComponentMode", parent=parent)
        self.triggered.connect(self.__do_it)
        self._latest_component_mode = dcc_core.Application.SelectionMode.POINTS
        app = dcc_core.Application.instance()
        self._mode_callback = app.register_event_callback(
            dcc_core.Application.EventType.SELECTION_MODE_CHANGED, self.__on_mode_change
        )
        self.setShortcut("F8")
        self.setShortcutContext(QtCore.Qt.ApplicationShortcut)

    def __on_mode_change(self):
        app = dcc_core.Application.instance()
        mode = app.get_selection_mode()
        settings = app.get_settings()
        if mode in self.COMPONENT_MODES:
            self._latest_component_mode = mode
            self._latest_selection_kind = settings.get_string(
                "session.viewport.select_tool.kind", ""
            )

    def __do_it(self):
        app = dcc_core.Application.instance()
        mode = app.get_selection_mode()
        settings = app.get_settings()
        if mode in self.COMPONENT_MODES:
            settings.set_string("session.viewport.select_tool.kind", "")
            app.set_selection_mode(dcc_core.Application.SelectionMode.PRIMS)
        else:
            settings.set_string("session.viewport.select_tool.kind", self._latest_selection_kind)
            app.set_selection_mode(self._latest_component_mode)


def selection_mode_button():
    selection_mode = QtWidgets.QToolButton()
    selection_mode.setPopupMode(QtWidgets.QToolButton.InstantPopup)
    selection_mode.setToolTip(i18n("toolbars.tools", "Selection Mode"))
    selection_mode.setIconSize(QtCore.QSize(16, 16))
    # selection_mode.setFixedSize(QtCore.QSize(20, 20))

    selection_mode_menu = QtWidgets.QMenu(selection_mode)
    selection_mode_group = QtWidgets.QActionGroup(selection_mode)

    label_map = {
        "Prim": i18n("toolbars.tools", "Prim"),
        "Point": i18n("toolbars.tools", "Point"),
        "Edge": i18n("toolbars.tools", "Edge"),
        "Face": i18n("toolbars.tools", "Face"),
        "Instance": i18n("toolbars.tools", "Instance"),
        "model": i18n("toolbars.tools", "model"),
        "group": i18n("toolbars.tools", "group"),
        "assembly": i18n("toolbars.tools", "assembly"),
        "component": i18n("toolbars.tools", "component"),
        "subcomponent": i18n("toolbars.tools", "subcomponent"),
    }

    def get_label(label):
        if label in label_map:
            return label_map[label]
        else:
            return label

    select_prims_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/select_prims"), get_label("Prim"), selection_mode_menu
    )
    select_prims_action.setData(dcc_core.Application.SelectionMode.PRIMS)
    select_prims_action.setCheckable(True)
    selection_mode_menu.addAction(select_prims_action)
    selection_mode_group.addAction(select_prims_action)

    selection_mode_menu.addSeparator()

    select_points_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/select_points"), get_label("Point"), selection_mode_menu
    )
    select_points_action.setData(dcc_core.Application.SelectionMode.POINTS)
    select_points_action.setCheckable(True)
    selection_mode_menu.addAction(select_points_action)
    selection_mode_group.addAction(select_points_action)

    select_edges_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/select_edges"), get_label("Edge"), selection_mode_menu
    )
    select_edges_action.setData(dcc_core.Application.SelectionMode.EDGES)
    select_edges_action.setCheckable(True)
    selection_mode_menu.addAction(select_edges_action)
    selection_mode_group.addAction(select_edges_action)

    select_faces_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/select_faces"), get_label("Face"), selection_mode_menu
    )
    select_faces_action.setData(dcc_core.Application.SelectionMode.FACES)
    select_faces_action.setCheckable(True)
    selection_mode_menu.addAction(select_faces_action)
    selection_mode_group.addAction(select_faces_action)

    selection_mode_menu.addSeparator()

    select_instances_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/select_instances"), get_label("Instance"), selection_mode_menu
    )
    select_instances_action.setData(dcc_core.Application.SelectionMode.INSTANCES)
    select_instances_action.setCheckable(True)
    selection_mode_menu.addAction(select_instances_action)
    selection_mode_group.addAction(select_instances_action)

    selection_mode_menu.addSeparator()

    icon_map = {
        "Point": ":/icons/select_points",
        "Edge": ":/icons/select_edges",
        "Face": ":/icons/select_faces",
        "Instance": ":/icons/select_instances",
        "Prim": ":/icons/select_prims",
        "model": ":/icons/select_models",
        "group": ":/icons/select_groups",
        "assembly": ":/icons/select_assemblies",
        "component": ":/icons/select_components",
        "subcomponent": ":/icons/select_subcomponents",
    }

    default_kinds = (
        Kind.Tokens.model,
        Kind.Tokens.group,
        Kind.Tokens.assembly,
        Kind.Tokens.component,
        Kind.Tokens.subcomponent,
    )

    for kind in default_kinds:
        kind_action = QtWidgets.QAction(
            QtGui.QIcon(icon_map[kind]), get_label(kind), selection_mode_menu
        )
        kind_action.setData(dcc_core.Application.SelectionMode.PRIMS)
        kind_action.setCheckable(True)
        selection_mode_menu.addAction(kind_action)
        selection_mode_group.addAction(kind_action)

    selection_mode_menu.addSeparator()

    all_kinds = Kind.Registry.GetAllKinds()

    custom_kinds = (x for x in all_kinds if x not in default_kinds)

    custom_kind_icon = ":/icons/select_components"

    for kind in custom_kinds:
        kind_action = QtWidgets.QAction(QtGui.QIcon(custom_kind_icon), kind, selection_mode_menu)
        kind_action.setData(dcc_core.Application.SelectionMode.PRIMS)
        kind_action.setCheckable(True)
        selection_mode_menu.addAction(kind_action)
        selection_mode_group.addAction(kind_action)

    select_prims_action.setChecked(True)

    selection_mode.setMenu(selection_mode_menu)

    app = dcc_core.Application.instance()

    tooltip = {
        dcc_core.Application.SelectionMode.PRIMS: i18n(
            "toolbars.tools", "Selection Mode (<b>Prim</b>)"
        ),
        dcc_core.Application.SelectionMode.POINTS: i18n(
            "toolbars.tools", "Selection Mode (<b>Point</b>)"
        ),
        dcc_core.Application.SelectionMode.EDGES: i18n(
            "toolbars.tools", "Selection Mode (<b>Edge</b>)"
        ),
        dcc_core.Application.SelectionMode.FACES: i18n(
            "toolbars.tools", "Selection Mode (<b>Face</b>)"
        ),
        dcc_core.Application.SelectionMode.INSTANCES: i18n(
            "toolbars.tools", "Selection Mode (<b>Instance</b>)"
        ),
    }

    select_prims_action.setChecked(True)
    selection_mode.setIcon(select_prims_action.icon())
    selection_mode.setToolTip(tooltip[dcc_core.Application.SelectionMode.PRIMS])

    def update_ui(mode, kind):
        if mode == dcc_core.Application.SelectionMode.PRIMS and kind:
            for action in selection_mode_group.actions():
                if action.text() == kind:
                    action.setChecked(True)
                    selection_mode.setIcon(action.icon())
                    selection_mode.setToolTip(
                        i18n("toolbars.tools", "Selection Mode (<b>%s</b>)") % kind
                    )
                    break
        else:
            for action in selection_mode_group.actions():
                if action.data() == mode:
                    action.setChecked(True)
                    selection_mode.setIcon(action.icon())
                    selection_mode.setToolTip(tooltip[mode])
                    break

    def change_mode():
        mode = app.get_selection_mode()
        current_kind = app.get_settings().get_string("session.viewport.select_tool.kind", "")

        selection_mode_group.blockSignals(True)

        update_ui(mode, current_kind)

        selection_mode_group.blockSignals(False)

    def setting_changed(setting_name, kind, change):
        mode = app.get_selection_mode()
        update_ui(mode, kind)

    app_callback = app.register_event_callback(
        dcc_core.Application.EventType.SELECTION_MODE_CHANGED, change_mode
    )

    settings = dcc_core.Application.instance().get_settings()
    settings.register_setting_changed("session.viewport.select_tool.kind", setting_changed)

    def mode_changed(action):
        mode = action.data()
        new_mode = mode != dcc_core.Application.instance().get_selection_mode()

        if int(mode) < int(dcc_core.Application.SelectionMode.COUNT) and (
            new_mode or action.text() == "Prim"
        ):
            settings.set_string("session.viewport.select_tool.kind", "")
            app.set_selection_mode(mode)
        else:
            settings.set_string("session.viewport.select_tool.kind", action.text())
            app.set_selection_mode(dcc_core.Application.SelectionMode.PRIMS)

    selection_mode_group.triggered.connect(mode_changed)

    return selection_mode


def init_toggle_selection_mode():
    main_win = dcc_core.Application.instance().get_main_window()

    toggle_mode_action = ToggleLatestComponentModeAction()
    main_win.addAction(toggle_mode_action)


def init_selection_mode_toolbar():
    main_win = dcc_core.Application.instance().get_main_window()

    init_toggle_selection_mode()

    selection_mode_toolbar = QtWidgets.QToolBar(i18n("toolbars.tools", "Selection Mode"))
    selection_mode_toolbar.setObjectName("selection_mode_toolbar")
    selection_mode_toolbar.setIconSize(QtCore.QSize(20, 20))

    selection_mode = selection_mode_button()
    selection_mode_toolbar.addWidget(selection_mode)

    main_win.addToolBar(QtCore.Qt.TopToolBarArea, selection_mode_toolbar)
