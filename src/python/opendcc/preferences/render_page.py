# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from __future__ import absolute_import
from Qt import QtWidgets, QtCore, QtGui
import opendcc.core as dcc_core

from opendcc.common_widgets import RolloutWidget
import opendcc.rendersystem as render_system

from opendcc.i18n import i18n


class RenderPage(QtWidgets.QWidget):
    ACTIVE_CONTROL_KEY = "render.active_control"

    def __init__(self, parent=None):
        super(RenderPage, self).__init__(parent=parent)
        self.setObjectName(i18n("preferences.render_page", "Render"))
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        rollout = RolloutWidget(i18n("preferences.render_page", "Render"), expandable=False)
        rollout_layout = QtWidgets.QFormLayout()
        rollout.set_layout(rollout_layout)
        layout.addWidget(rollout)
        layout.addStretch()
        self.setLayout(layout)

        self._active_render = QtWidgets.QComboBox()
        controls = render_system.RenderControlHub.instance().get_controls()
        for key, val in controls.items():
            self._active_render.addItem(key)
        rollout_layout.addRow(
            i18n("preferences.render_page", "Active render control:"), self._active_render
        )
        self.restore_previous_settings()
        self._active_render.currentTextChanged.connect(self._render_control_changed)

    def _render_control_changed(self, control_key):
        controls = render_system.RenderControlHub.instance().get_controls()
        if control_key in controls.keys():
            render_system.RenderSystem.instance().set_render_control(controls[control_key])

    def save_settings(self):
        settings = dcc_core.Application.instance().get_settings()
        value = self._active_render.currentText()
        settings.set_string(self.ACTIVE_CONTROL_KEY, value)

    def restore_previous_settings(self):
        settings = dcc_core.Application.instance().get_settings()
        config = dcc_core.Application.instance().get_app_config()
        self._active_render.setCurrentText(
            settings.get_string(
                self.ACTIVE_CONTROL_KEY, config.get_string(self.ACTIVE_CONTROL_KEY, "")
            )
        )
