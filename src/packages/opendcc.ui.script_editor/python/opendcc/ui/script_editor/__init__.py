# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from opendcc.packaging import PackageEntryPoint
import opendcc.core as dcc_core
from opendcc.i18n import i18n
from Qt import QtWidgets


class ScriptEditorEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        from .script_editor import ScriptEditor

        # TODO: ideally this should be in `AFTER_UI_LOAD` callback, but Panels menu currently doesn't receive updates after startup
        if QtWidgets.QApplication.instance():
            registry = dcc_core.PanelFactory.instance()
            registry.register_panel(
                "script_editor",
                lambda: ScriptEditor(),
                i18n("panels", "Script Editor"),
                True,
                ":/icons/panel_script_editor",
            )
        app = dcc_core.Application.instance()
        app.register_event_callback(app.EventType.AFTER_UI_LOAD, lambda: self.init_ui())

    def init_ui(self):
        from opendcc.preferences_widget import PreferencePageFactory
        from .pref_page import ScriptEditorPage

        PreferencePageFactory.register_preference_page("Script Editor", lambda: ScriptEditorPage())
