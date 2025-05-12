# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from __future__ import absolute_import
from Qt import QtWidgets, QtCore, QtGui
from opendcc.ui.code_editor.code_editor import CodeEditor
from opendcc.ui.script_editor.script_editor import ScriptEditor
import opendcc.core as dcc_core
from opendcc.common_widgets import RolloutWidget

from opendcc.i18n import i18n


class ScriptEditorPage(QtWidgets.QWidget):
    _enable_autocomplete_key = "scripteditor.enable_autocomplete"
    _save_tabs_before_execute_key = "scripteditor.save_before_execute"

    def __init__(self, parent=None):
        super(ScriptEditorPage, self).__init__(parent=parent)
        self.setObjectName(i18n("preferences.script_editor", "Script Editor"))
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        rollout = RolloutWidget(
            i18n("preferences.script_editor", "Script Editor"), expandable=False
        )
        rollout_layout = QtWidgets.QFormLayout()
        rollout.set_layout(rollout_layout)
        layout.addWidget(rollout)
        layout.addStretch()
        self.setLayout(layout)
        self._enable_autocomplete = QtWidgets.QCheckBox()
        self._save_tabs_before_execute = QtWidgets.QCheckBox()
        rollout_layout.addRow(
            i18n("preferences.script_editor", "Enable autocomplete:"), self._enable_autocomplete
        )
        rollout_layout.addRow(
            i18n("preferences.script_editor", "Save tabs before execution:"),
            self._save_tabs_before_execute,
        )
        self.restore_previous_settings()

        self._enable_autocomplete.stateChanged.connect(self._set_enable_autocomplete)
        self._save_tabs_before_execute.stateChanged.connect(self._set_save_tabs_before_execute)

    def _set_enable_autocomplete(self, state):
        value = True if state == QtCore.Qt.Checked else False
        CodeEditor.setEnableAutocomplete(value)

    def _set_save_tabs_before_execute(self, state):
        value = True if state == QtCore.Qt.Checked else False
        ScriptEditor.setSaveTabs(value)

    def save_settings(self):
        settings = dcc_core.Application.instance().get_settings()
        enable_autocomplete_value = (
            True if self._enable_autocomplete.checkState() == QtCore.Qt.Checked else False
        )
        save_tabs_before_execute_value = (
            True if self._save_tabs_before_execute.checkState() == QtCore.Qt.Checked else False
        )
        settings.set_bool(self._enable_autocomplete_key, enable_autocomplete_value)
        settings.set_bool(self._save_tabs_before_execute_key, save_tabs_before_execute_value)

    def restore_previous_settings(self):
        settings = dcc_core.Application.instance().get_settings()
        self._enable_autocomplete.setChecked(settings.get_bool(self._enable_autocomplete_key, True))
        self._save_tabs_before_execute.setChecked(
            settings.get_bool(self._save_tabs_before_execute_key, True)
        )
