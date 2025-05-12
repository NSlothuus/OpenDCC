# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.core as dcc_core
from opendcc.packaging import PackageEntryPoint


class HydraOpNodeEditorEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        from Qt import QtWidgets
        from opendcc.ui import node_editor
        from .node_editor import HydraOpNodeEditorWidget
        from . import hydra_op_node_editor
        from opendcc.i18n import i18n
        from pxr import Plug

        Plug.Registry().RegisterPlugins(package.get_root_dir() + "/pxr_plugins")
        if not QtWidgets.QApplication.instance():
            return

        def hydra_op_node_editor_widget():
            editor_name = "HydraOp"
            model = hydra_op_node_editor.HydraOpGraphModel()
            item_registry = hydra_op_node_editor.HydraOpItemRegistry(model)
            scene = node_editor.NodeEditorScene(model, item_registry, None)
            view = node_editor.NodeEditorView(scene)
            return HydraOpNodeEditorWidget(
                editor_name, model, view, scene, hydra_op_node_editor, item_registry
            )

        registry = dcc_core.PanelFactory.instance()
        registry.register_panel(
            "hydra_op_node_editor",
            hydra_op_node_editor_widget,
            i18n("panels", "HydraOp Node Editor"),
            False,
            ":icons/panel_hydraop_node_editor",
            "Hydra",
        )
