# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.core as dcc_core
from opendcc.i18n import i18n
from opendcc.packaging import PackageEntryPoint
from Qt import QtWidgets, QtCore, QtGui

# Moved out of ToolContextActions due to a translate-parser bug
tool_select = i18n("toolbars.tools", "Select Tool")


class HydraOpEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        import os

        app = dcc_core.Application.instance()
        app.register_event_callback(app.EventType.AFTER_UI_LOAD, lambda: self.init_hydra_op_ui())

    def init_hydra_op_ui(self):
        from opendcc.actions.tool_context_actions import ToolContextActions
        from opendcc.actions.tool_settings_button import ToolSettingsButton

        ToolContextActions().register_action_group("HydraOp")
        ToolContextActions().register_action(
            "HydraOp", "SelectTool", tool_select, ":icons/select_tool.png", "Q"
        )
        app = dcc_core.Application.instance()

        mw = app.get_main_window()
        tool_context_toolbar = mw.findChild(QtWidgets.QToolBar, "tools_toolbar")
        if tool_context_toolbar:
            hydra_op_select_action = ToolContextActions().get_action("HydraOp", "SelectTool")
            hydra_op_actions = [
                hydra_op_select_action,
            ]

            tool_context_toolbar_actions = tool_context_toolbar.actions()

            for action in hydra_op_actions:
                tool_button = ToolSettingsButton(action)
                if tool_context_toolbar_actions:
                    parent_action = tool_context_toolbar.insertWidget(
                        tool_context_toolbar_actions[0], tool_button
                    )
                    tool_button.set_parent_action(parent_action)
                else:
                    parent_action = tool_context_toolbar.addWidget(tool_button)
                    tool_button.set_parent_action(parent_action)
                action.setVisible(False)

        icon_registry = dcc_core.NodeIconRegistry.instance()
        icon_registry.register_icon(
            "USD", "HydraOpUsdIn", ":/icons/hydraopusdin", ":/icons/node_editor/hydraopusdin"
        )
        icon_registry.register_icon(
            "USD",
            "HydraOpSetAttribute",
            ":/icons/hydraopsetattribute",
            ":/icons/node_editor/hydraopsetattribute",
        )
        icon_registry.register_icon(
            "USD", "HydraOpMerge", ":/icons/hydraopmerge", ":/icons/node_editor/hydraopmerge"
        )
        icon_registry.register_icon(
            "USD", "HydraOpPrune", ":/icons/hydraopprune", ":/icons/node_editor/hydraopprune"
        )
        icon_registry.register_icon(
            "USD",
            "HydraOpIsolate",
            ":/icons/hydraopisolate",
            ":/icons/node_editor/hydraopisolate",
        )
        icon_registry.register_icon(
            "USD",
            "HydraOpNodegraph",
            ":/icons/hydraopnodegraph",
            ":/icons/node_editor/hydraopnodegraph",
        )
        icon_registry.register_icon(
            "USD", "HydraOpGroup", ":/icons/hydraopgroup", ":/icons/node_editor/hydraopgroup"
        )

        icon_registry.register_icon("HydraOp", "subdmesh", ":/icons/mesh.png")
        icon_registry.register_icon("HydraOp", "polymesh", ":/icons/mesh.png")
        icon_registry.register_icon("HydraOp", "options", ":/icons/attributes.png")
        icon_registry.register_icon("HydraOp", "camera", ":/icons/camera.png")
        icon_registry.register_icon("HydraOp", "material", ":/icons/Textured.png")
        icon_registry.register_icon("HydraOp", "light", ":/icons/light_default.png")
        icon_registry.register_icon("HydraOp", "distant_light", ":/icons/light_distant.png")
        icon_registry.register_icon("HydraOp", "cylinder_light", ":/icons/light_cylinder.png")
        icon_registry.register_icon("HydraOp", "disk_light", ":/icons/light_disk.png")
        icon_registry.register_icon("HydraOp", "quad_light", ":/icons/light_quad.png")
        icon_registry.register_icon("HydraOp", "skydome_light", ":/icons/light_environment.png")
        icon_registry.register_icon("HydraOp", "point_light", ":/icons/light_sphere.png")
        icon_registry.register_icon("HydraOp", "xform", ":/icons/xform.png")
        icon_registry.register_icon("HydraOp", "instance", ":/icons/instance.png")
        icon_registry.register_icon("HydraOp", "group", ":/icons/group.png")
        icon_registry.register_icon("HydraOp", "asset", ":/icons/asset.png")

        icon_registry.register_icon("small_HydraOp", "subdmesh", ":/icons/small_mesh")
        icon_registry.register_icon("small_HydraOp", "polymesh", ":/icons/small_mesh")
        icon_registry.register_icon("small_HydraOp", "light", ":/icons/small_light")
        icon_registry.register_icon("small_HydraOp", "group", ":/icons/small_group")
        icon_registry.register_icon("small_HydraOp", "asset", ":/icons/small_asset")
        icon_registry.register_icon("small_HydraOp", "instance", ":/icons/small_instance")
        icon_registry.register_icon("small_HydraOp", "xform", ":/icons/small_xform")
