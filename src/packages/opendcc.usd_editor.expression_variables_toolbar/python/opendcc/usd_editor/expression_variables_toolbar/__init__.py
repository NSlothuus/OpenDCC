# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from opendcc.packaging import PackageEntryPoint
import opendcc.core as dcc_core
from pxr import Plug
import os


class ExpressionVariablesToolbarEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        Plug.Registry().RegisterPlugins(os.path.join(package.get_root_dir(), "pxr_plugins"))

        app = dcc_core.Application.instance()
        app.register_event_callback(
            app.EventType.AFTER_UI_LOAD, lambda: self.init_expression_variables_toolbar()
        )

    def init_expression_variables_toolbar(self):
        from opendcc.usd_editor.expression_variables_toolbar.expression_variables_toolbar import (
            ExpressionVariablesToolbar,
        )

        toolbar = ExpressionVariablesToolbar()
