# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.usd_editor.bullet_physics as bullet
import numpy as np
from pxr import Usd, Vt, Gf, UsdGeom
import opendcc.core as dcc_core
import numpy as np


def add_shift(prim, shift, attr_name):
    points_attribute = prim.GetAttribute(attr_name)
    P = points_attribute.Get()
    np_P = np.array(P)
    shift = np.array(shift)

    for i in range(3):
        np_P[:, i] = np_P[:, i] + shift[i]
    new_P = Vt.Vec3fArray.FromNumpy(np_P)
    points_attribute.Set(new_P)


def add_shift_to_prim(prim, shift):
    add_shift(prim, shift, "points")
    add_shift(prim, shift, "extent")
    xform_cache = UsdGeom.XformCache()
    tr, _ = xform_cache.GetLocalTransformation(prim)
    w_shift = tr.Transform(Gf.Vec3d(shift))
    t = prim.GetAttribute("xformOp:translate").Get()
    if t:
        prim.GetAttribute("xformOp:translate").Set(2 * t - w_shift)
    else:
        xf = UsdGeom.XformCommonAPI(prim)
        if xf:
            xf.SetTranslate(-w_shift)


def normalize_extant_shift(stage, paths):
    for path in paths:
        prim = stage.GetPrimAtPath(path)
        if not prim:
            continue
        bullet.reset_pivots([path])
        mesh = UsdGeom.Mesh(prim)
        if not mesh:
            continue
        extent = mesh.GetExtentAttr().Get()
        if extent is None:
            continue
        center = (extent[1] + extent[0]) / 2
        shift = -center
        add_shift_to_prim(prim, shift)


def reset_pivots_and_normalize_extents_shift_in_selected_objects(clear_extant_shift):
    app = dcc_core.Application.instance()
    session = app.get_session()
    stage = session.get_current_stage()
    selection = app.get_selection().get_fully_selected_paths()

    if len(selection) > 0:
        with dcc_core.UsdEditsUndoBlock():
            if clear_extant_shift:
                paths = bullet.get_supported_prims_paths_recursively(stage, selection)
                normalize_extant_shift(stage, paths)
            else:
                bullet.reset_pivots(selection)
