# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.core as dcc_core
from pxr import UsdGeom, Sdf
import opendcc.core as dcc_core
from opendcc.undo import UsdEditsUndoBlock

from opendcc.stage_utils import (
    find_new_name_for_prim,
    has_parent_same_child,
)


def create_point_instancer_context():
    current_context = dcc_core.Application.instance().get_current_viewport_tool()

    if not current_context or current_context.get_name() != "PointInstancer":
        tool_context = dcc_core.ViewportToolContextRegistry.create_tool_context(
            "USD", "PointInstancer"
        )
        dcc_core.Application.instance().set_current_viewport_tool(tool_context)


def create_point_instancer():
    app = dcc_core.Application.instance()
    stage = app.get_session().get_current_stage()

    def create_prim(stage, parent_path, name, prim_type):
        root_prim = stage.GetPrimAtPath(parent_path)
        new_name = name + "1"
        new_path = parent_path.AppendChild(new_name)
        if has_parent_same_child(root_prim, new_name):
            new_path = find_new_name_for_prim(new_path, root_prim)
        return stage.DefinePrim(new_path, prim_type)

    with UsdEditsUndoBlock():
        create_prim(stage, Sdf.Path.absoluteRootPath, "point_instancer", "PointInstancer")
