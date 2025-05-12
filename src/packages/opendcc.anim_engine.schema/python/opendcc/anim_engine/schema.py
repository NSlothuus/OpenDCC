# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from opendcc.packaging import PackageEntryPoint


class AnimEngineSchemaEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        from pxr import Plug
        import os

        Plug.Registry().RegisterPlugins(os.path.join(package.get_root_dir(), "pxr_plugins"))
