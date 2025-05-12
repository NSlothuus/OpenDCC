# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtCore, QtGui
import opendcc.core as dcc_core


class ToolSettingsButton(QtWidgets.QToolButton):
    def __init__(self, action, parent=None):
        super(ToolSettingsButton, self).__init__(parent=parent)
        self.setDefaultAction(action)
        self.setToolButtonStyle(QtCore.Qt.ToolButtonIconOnly)

        # After adding a QToolButton to a QToolBar,
        # Qt creates a secondary QAction
        # that remains even when the default action is hidden.
        # The QToolButton then gets disabled.
        self.parent_action = None
        self.default_action = action
        action.changed.connect(self.check_changed)

    def mouseDoubleClickEvent(self, event):
        dcc_core.create_panel("tool_settings")

    def set_parent_action(self, parent_action):
        self.parent_action = parent_action

    def check_changed(self):
        if self.parent_action:
            action_visibility = self.default_action.isVisible()
            parent_visibility = self.parent_action.isVisible()
            if action_visibility != parent_visibility:
                self.parent_action.setVisible(action_visibility)
