# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from pxr import Tf, Usd, Sdf
import re
import opendcc.core as dcc_core


def open_stage(path: str) -> None:
    """
    Open USD Stage with options and set the stage current
    """

    root_layer = Sdf.Layer.Find(path)

    if root_layer:
        root_layer.Reload()
    else:
        root_layer = Sdf.Layer.FindOrOpen(path)

    if root_layer:
        app = dcc_core.Application.instance()
        session = app.get_session()
        cache = session.get_stage_cache()

        settings = app.get_settings()
        load_operation_setting = settings.get_string(
            "open_stage_command.load_operation", "load_all"
        )
        session_layer = settings.get_string("new_stage_command.default_edit_target", "root_layer")
        create_session_layer = settings.get_bool("new_stage_command.create_session_layer", True)

        load_operation = (
            Usd.Stage.LoadAll if load_operation_setting == "load_all" else Usd.Stage.LoadNone
        )

        with Usd.StageCacheContext(cache):
            if create_session_layer:
                stage = Usd.Stage.Open(root_layer, load_operation)
                session.set_current_stage(stage)
                if session_layer == "root_layer":
                    stage.SetEditTarget(stage.GetRootLayer())
                else:
                    stage.SetEditTarget(stage.GetSessionLayer())
            else:
                stage = Usd.Stage.Open(root_layer, load=load_operation, sessionLayer=None)
                session.set_current_stage(stage)


def new_stage() -> Usd.Stage:
    """
    Create New USD Stage with default options and add to Session
    """

    settings = dcc_core.Application.instance().get_settings()
    session_layer = settings.get_string("new_stage_command.default_edit_target", "root_layer")
    create_session_layer = settings.get_bool("new_stage_command.create_session_layer", True)

    app = dcc_core.Application.instance()
    session = app.get_session()
    layer = Sdf.Layer.CreateAnonymous(".usda")

    stage = None
    if create_session_layer:
        stage = Usd.Stage.Open(layer)
        if session_layer == "root_layer":
            stage.SetEditTarget(stage.GetRootLayer())
        else:
            stage.SetEditTarget(stage.GetSessionLayer())
    else:
        stage = Usd.Stage.Open(layer, sessionLayer=None)

    if settings.get_bool("new_stage_command.default_up_axis_enable", True):
        stage.SetMetadata("upAxis", settings.get_string("new_stage_command.default_up_axis", "Y"))
    if settings.get_bool("new_stage_command.default_animation_rate_enable", False):
        stage.SetFramesPerSecond(
            settings.get_double("new_stage_command.default_animation_rate", 24.0)
        )
    if settings.get_bool("new_stage_command.default_meters_per_unit_enable", False):
        stage.SetMetadata(
            "metersPerUnit",
            settings.get_double("new_stage_command.default_meters_per_unit", 0.01),
        )
    if settings.get_bool("new_stage_command.default_frame_range_enable", True):
        stage.SetStartTimeCode(settings.get_double("new_stage_command.default_frame_range0", 1.0))
        stage.SetEndTimeCode(settings.get_double("new_stage_command.default_frame_range1", 100.0))
    if settings.get_bool("new_stage_command.default_prim_enable", False):
        default_prim = settings.get_string("new_stage_command.default_prim", "world")
        stage.SetDefaultPrim(
            stage.DefinePrim(Sdf.Path.absoluteRootPath.AppendChild(default_prim), "Scope")
        )

    session.set_current_stage(stage)
    return stage


def reload_stage(stage: Usd.Stage) -> None:
    """
    Reload Stage with clearing refine levels
    """
    session = dcc_core.Application.instance().get_session()
    dcc_core.UsdViewportRefineManager.instance().clear_stage(session.get_current_stage_id())
    stage.Reload()


def has_parent_same_child(parent_prim: Usd.Prim, child_name: str) -> bool:
    child = parent_prim.GetChild(child_name)
    return child.IsValid()


def find_new_name_for_prim(path: Sdf.Path, parent_prim: Usd.Prim) -> Sdf.Path:
    parent_path = path.GetParentPath()
    prim_name = path.name

    # Regex to match trailing digits
    suffix_re = re.compile(r"(\D*)(\d*)$")
    base_name_match = suffix_re.match(prim_name)
    base_name = base_name_match.group(1) if base_name_match else prim_name

    max_suffix = 0
    for child in parent_prim.GetAllChildren():
        match = suffix_re.match(child.GetName())
        if match and match.group(1) == base_name:
            suffix = match.group(2)
            if suffix.isdigit():
                max_suffix = max(max_suffix, int(suffix))

    new_name = f"{base_name}{max_suffix + 1}"
    return parent_path.AppendChild(new_name)


def apply_schema(schema_name, prims):
    """
    If specification of schema exists and loaded and spec has attributes,
    create such attributes on prims and fill metadata allowedTokens
    """
    schema_type = Usd.SchemaRegistry.GetAPITypeFromSchemaTypeName(schema_name)

    if schema_type:
        for prim in prims:
            # apply schema on prim
            prim.ApplyAPI(schema_type)


if Usd.GetVersion() >= (0, 20, 5):

    def apply_schema_to_spec(schema_name, prim_specs):
        """
        If specification of schema exists and loaded and spec has attributes,
        create such attributes on prims and fill metadata allowedTokens
        """
        schema_type = Tf.Type.FindByName(schema_name)
        schema_type_name = Usd.SchemaRegistry().GetSchemaTypeName(schema_type)

        if not schema_type or not schema_type_name:
            return

        if schema_type.pythonClass:
            for prim_spec in prim_specs:
                item_list = prim_spec.GetInfo("apiSchemas")
                item_list.prependedItems += [schema_type_name]
                prim_spec.SetInfo("apiSchemas", item_list)

else:

    def apply_schema_to_spec(schema_name, prim_specs):
        """
        If specification of schema exists and loaded and spec has attributes,
        create such attributes on prims and fill metadata allowedTokens
        """
        schema_type = Tf.Type.FindByName(schema_name)
        schema_spec = Usd.SchemaRegistry.GetPrimDefinition(schema_name)

        if not schema_type or not schema_spec:
            return

        if schema_type.pythonClass:
            for prim_spec in prim_specs:
                item_list = prim_spec.GetInfo("apiSchemas")
                item_list.prependedItems += [schema_spec.name]
                prim_spec.SetInfo("apiSchemas", item_list)
