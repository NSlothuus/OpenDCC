# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from __future__ import absolute_import
from Qt import QtWidgets, QtCore, QtGui
import opendcc.core as dcc_core
from opendcc.common_widgets import DoubleWidget, RolloutWidget
from opendcc.i18n import i18n


class UndoPage(QtWidgets.QWidget):
    _enable_key = "undo.enable"
    _finite_key = "undo.finite"
    _stack_size_key = "undo.stack_size"

    def __init__(self, parent=None):
        super(UndoPage, self).__init__(parent=parent)
        self.setObjectName(i18n("preferences.undo", "Undo"))
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        rollout = RolloutWidget(i18n("preferences.undo", "Undo"), expandable=False)
        rollout_layout = QtWidgets.QFormLayout()
        rollout.set_layout(rollout_layout)
        layout.addWidget(rollout)
        layout.addStretch()
        self.setLayout(layout)
        self.layout().setAlignment(QtCore.Qt.AlignTop | QtCore.Qt.AlignLeft)
        self._enable_undo = QtWidgets.QCheckBox()
        self._finite_undo = QtWidgets.QCheckBox()
        self._stack_size = DoubleWidget(as_int=True)
        self._stack_size.set_range(1, 10000)
        self._stack_size.enable_slider(1, 200)
        rollout_layout.addRow(i18n("preferences.undo", "Enable:"), self._enable_undo)
        rollout_layout.addRow(i18n("preferences.undo", "Finite:"), self._finite_undo)
        rollout_layout.addRow(i18n("preferences.undo", "Stack Size:"), self._stack_size)
        self.restore_previous_settings()

        self._enable_undo.stateChanged.connect(self._set_enable_undo)
        self._finite_undo.stateChanged.connect(self._set_finite_undo)
        self._stack_size.value_changed.connect(self._set_stack_size)

    def _set_enable_undo(self, state):
        value = True if state == QtCore.Qt.Checked else False
        dcc_core.Application.instance().get_undo_stack().set_enabled(value)

    def _set_finite_undo(self, state):
        value = True if state == QtCore.Qt.Checked else False
        dcc_core.Application.instance().get_undo_stack().set_undo_limit(
            int(self._stack_size.get_value()) if value else 0
        )

    def _set_stack_size(self, value):
        if self._finite_undo.checkState() == QtCore.Qt.Checked:
            dcc_core.Application.instance().get_undo_stack().set_undo_limit(
                int(self._stack_size.get_value())
            )

    def save_settings(self):
        settings = dcc_core.Application.instance().get_settings()
        enable_key_value = True if self._enable_undo.checkState() == QtCore.Qt.Checked else False
        finite_undo_value = True if self._finite_undo.checkState() == QtCore.Qt.Checked else False
        stack_size_value = self._stack_size.get_value()
        settings.set_bool(self._enable_key, enable_key_value)
        settings.set_bool(self._finite_key, finite_undo_value)
        settings.set_int(self._stack_size_key, int(stack_size_value))

    def restore_previous_settings(self):
        settings = dcc_core.Application.instance().get_settings()
        self._enable_undo.setChecked(settings.get_bool(self._enable_key, True))
        self._finite_undo.setChecked(settings.get_bool(self._finite_key, True))
        self._stack_size.set_value(settings.get_int(self._stack_size_key, 100))
