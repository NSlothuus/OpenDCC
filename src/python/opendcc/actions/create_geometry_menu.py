# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtGui, QtCore
from .create_prim import CreatePrimAction
from .custom_widget_action import CustomAction
from .create_mesh import plane, sphere
from .options_widget import OptionsWidget, make_option

from opendcc.i18n import i18n


class CreatePlane(CustomAction):
    def __init__(self, parent=None):
        CustomAction.__init__(self, i18n("main_menu.create", "Plane"), parent)
        self.setIcon(QtGui.QIcon(":icons/mesh"))
        self.on_checkbox_clicked(self.show_options)
        self.custom_triggered.connect(self.do)

        options = []
        options.append(
            make_option(
                i18n("main_menu.create", "Width"),
                "create_plane_command.width",
                {"type": "float"},
                1.0,
            )
        )
        options.append(
            make_option(
                i18n("main_menu.create", "Depth"),
                "create_plane_command.depth",
                {"type": "float"},
                1.0,
            )
        )
        options.append(
            make_option(
                i18n("main_menu.create", "Segments U"),
                "create_plane_command.seg_u",
                {"type": "int", "min": 1, "max": 9001},
                4,
            )
        )
        options.append(
            make_option(
                i18n("main_menu.create", "Segments V"),
                "create_plane_command.seg_v",
                {"type": "int", "min": 1, "max": 9001},
                4,
            )
        )

        self.options_widget = OptionsWidget(
            i18n("main_menu.create", "Create Plane"),
            i18n("main_menu.create", "Create"),
            self.do,
            options,
            self.parent(),
        )

    def do(self):
        plane()

    def show_options(self):
        self.options_widget.show()


class CreateSphere(CustomAction):
    def __init__(self, parent=None):
        CustomAction.__init__(self, i18n("main_menu.create", "Sphere"), parent)
        self.setIcon(QtGui.QIcon(":icons/sphere_mesh"))
        self.on_checkbox_clicked(self.show_options)
        self.custom_triggered.connect(self.do)

        options = []
        options.append(
            make_option(
                i18n("main_menu.create", "Radius"),
                "create_sphere_command.radius",
                {"type": "float"},
                1.0,
            )
        )
        options.append(
            make_option(
                i18n("main_menu.create", "Segments U"),
                "create_sphere_command.seg_u",
                {"type": "int", "min": 3, "max": 9001},
                16,
            )
        )
        options.append(
            make_option(
                i18n("main_menu.create", "Segments V"),
                "create_sphere_command.seg_v",
                {"type": "int", "min": 3, "max": 9001},
                16,
            )
        )

        self.options_widget = OptionsWidget(
            i18n("main_menu.create", "Create Sphere"),
            i18n("main_menu.create", "Create"),
            self.do,
            options,
            self.parent(),
        )

    def do(self):
        sphere()

    def show_options(self):
        self.options_widget.show()


def create_geometry_actions(parent, custom=False):
    actions = []

    if custom:
        actions.append(CreatePlane(parent))
        actions.append(CreateSphere(parent))
    else:
        plane_action = QtWidgets.QAction(
            QtGui.QIcon(":icons/mesh"), i18n("main_menu.create", "Plane"), parent
        )
        plane_action.triggered.connect(plane)
        actions.append(plane_action)
        sphere_action = QtWidgets.QAction(
            QtGui.QIcon(":icons/sphere_mesh"), i18n("main_menu.create", "Sphere"), parent
        )
        sphere_action.triggered.connect(sphere)
        actions.append(sphere_action)

    separator = QtWidgets.QAction(parent)
    separator.setSeparator(True)
    actions.append(separator)

    def create_prim_action(prim_name, prim_type, icon, label, parent):
        actions.append(
            CreatePrimAction(
                prim_name,
                prim_type,
                icon=QtGui.QIcon(icon),
                label=label,
                parent=parent,
            )
        )

    create_prim_action("Cube", "Cube", ":icons/cube", i18n("main_menu.create", "Cube"), parent)
    create_prim_action(
        "Sphere", "Sphere", ":icons/sphere", i18n("main_menu.create", "Sphere"), parent
    )
    create_prim_action(
        "Cylinder", "Cylinder", ":icons/cylinder", i18n("main_menu.create", "Cylinder"), parent
    )
    create_prim_action("Cone", "Cone", ":icons/cone", i18n("main_menu.create", "Cone"), parent)
    create_prim_action(
        "Capsule", "Capsule", ":icons/capsule", i18n("main_menu.create", "Capsule"), parent
    )
    return actions


class CreateGeometryMenu(QtWidgets.QMenu):
    def __init__(self, parent=None):
        QtWidgets.QMenu.__init__(self, i18n("main_menu.create", "Geometry"), parent)
        self.setIcon(QtGui.QIcon(":/icons/mesh"))

        actions = create_geometry_actions(self, custom=True)
        if actions:
            self.addActions(actions)
