# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from ._anim_engine_core import *
from opendcc.packaging import PackageEntryPoint
import opendcc.core as dcc_core

from pxr import Plug
import os


class AnimEngineEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        app = dcc_core.Application.instance()
        # TODO: moved it to anim_engine core?
        Plug.Registry().RegisterPlugins(os.path.join(package.get_root_dir(), "pxr_plugins"))
