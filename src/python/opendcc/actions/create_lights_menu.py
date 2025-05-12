# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtGui
from pxr import Tf, Usd
from .create_prim import CreateLightPrimAction

from opendcc.i18n import i18n

LIGHT_CYLINDER = i18n("main_menu.create", "Cylinder Light")
LIGHT_DISK = i18n("main_menu.create", "Disk Light")
LIGHT_DISTANT = i18n("main_menu.create", "Distant Light")
LIGHT_DOME = i18n("main_menu.create", "Dome Light")
LIGHT_RECT = i18n("main_menu.create", "Rect Light")
LIGHT_SPHERE = i18n("main_menu.create", "Sphere Light")
LIGHT_DEFAULT = i18n("main_menu.create", "default")

LIGHT_ICONS = [
    ("UsdLuxCylinderLight", QtGui.QIcon(":/icons/light_cylinder.png"), LIGHT_CYLINDER),
    ("UsdLuxDiskLight", QtGui.QIcon(":/icons/light_disk.png"), LIGHT_DISK),
    ("UsdLuxDistantLight", QtGui.QIcon(":/icons/light_distant.png"), LIGHT_DISTANT),
    ("UsdLuxDomeLight", QtGui.QIcon(":/icons/light_environment.png"), LIGHT_DOME),
    ("UsdLuxRectLight", QtGui.QIcon(":/icons/light_quad.png"), LIGHT_RECT),
    ("UsdLuxSphereLight", QtGui.QIcon(":/icons/light_sphere.png"), LIGHT_SPHERE),
    ("default", QtGui.QIcon(":/icons/light_default.png"), LIGHT_DEFAULT),
]


if Usd.GetVersion() >= (0, 20, 5):

    def create_light_actions(parent):
        actions = []
        for usd_type_name, icon, label in LIGHT_ICONS:
            schema_type = Tf.Type.FindByName(usd_type_name)
            if not schema_type:
                continue
            schema_type_name = Usd.SchemaRegistry().GetSchemaTypeName(schema_type)
            if not schema_type_name:
                continue
            actions.append(
                CreateLightPrimAction(
                    schema_type_name, schema_type_name, icon=icon, label=label, parent=parent
                )
            )
        return actions

else:

    def create_light_actions(parent):
        actions = []
        for usd_type_name, icon in LIGHT_ICONS:
            schema_type = Tf.Type.FindByName(usd_type_name)
            if not schema_type:
                continue
            spec = Usd.SchemaRegistry.GetPrimDefinition(schema_type)
            if not spec:
                continue
            actions.append(
                CreateLightPrimAction(spec.typeName, spec.typeName, icon=icon, parent=parent)
            )
        return actions


class CreateLightsMenu(QtWidgets.QMenu):
    def __init__(self, parent=None):
        QtWidgets.QMenu.__init__(self, i18n("main_menu.create", "Lights"), parent)
        self.setIcon(QtGui.QIcon(":/icons/light_default.png"))

        actions = create_light_actions(self)
        if actions:
            self.addActions(actions)
