# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from pxr import Tf

Tf.PreparePythonModule()
del Tf

from opendcc.packaging import PackageEntryPoint
from .material_editor_widget import MaterialEditorWidget
from opendcc.usd_editor import material_editor
from opendcc.ui.node_editor import _node_editor as node_editor
from opendcc.i18n import i18n


class MaterialNodeEditorEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        from opendcc.i18n import i18n
        from Qt import QtWidgets
        import opendcc.core as dcc_core

        if not QtWidgets.QApplication.instance():
            return

        def material_editor_widget_init():
            editor_name = "Material"
            model = material_editor.MaterialGraphModel()
            item_registry = material_editor.MaterialEditorItemRegistry(model)
            scene = node_editor.NodeEditorScene(model, item_registry, None)
            view = node_editor.NodeEditorView(scene)
            return MaterialEditorWidget(
                editor_name, model, view, scene, material_editor, item_registry
            )

        registry = dcc_core.PanelFactory.instance()
        registry.register_panel(
            "material_editor",
            material_editor_widget_init,
            i18n("panels", "Material Editor"),
            True,
            ":icons/panel_node_editor",
        )
