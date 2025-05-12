# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from opendcc.packaging import PackageEntryPoint
import opendcc.core as dcc_core


class LiveShareEntryPoint(PackageEntryPoint):
    def initialize(self, package):
        app = dcc_core.Application.instance()
        app.register_event_callback(app.EventType.AFTER_UI_LOAD, lambda: self.init_ui())

    def init_ui(self):
        from opendcc.preferences_widget import PreferencePageFactory
        from opendcc.usd_editor.live_share.pref_ui import LiveSharePage

        PreferencePageFactory.register_preference_page("LiveShare", lambda: LiveSharePage())

        from .widgets import init_live_share_toolbar

        init_live_share_toolbar()
