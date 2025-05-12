# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.core as dcc_core
from opendcc import cmds


def plane():
    settings = dcc_core.Application.instance().get_settings()
    width = settings.get_double("create_plane_command.width", 1.0)
    depth = settings.get_double("create_plane_command.depth", 1.0)
    seg_u = settings.get_int("create_plane_command.seg_u", 4)
    seg_v = settings.get_int("create_plane_command.seg_v", 4)
    cmds.create_mesh("Plane", "plane", width=width, depth=depth, u_segments=seg_u, v_segments=seg_v)


def sphere():
    settings = dcc_core.Application.instance().get_settings()
    radius = settings.get_double("create_sphere_command.radius", 1.0)
    seg_u = settings.get_int("create_sphere_command.seg_u", 16)
    seg_v = settings.get_int("create_sphere_command.seg_v", 16)
    cmds.create_mesh("Sphere", "sphere", radius=radius, u_segments=seg_u, v_segments=seg_v)
