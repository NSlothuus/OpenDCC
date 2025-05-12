# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtCore, QtGui
import opendcc.core as dcc_core

from opendcc.common_widgets import RolloutWidget

from opendcc.i18n import i18n


dark_theme = i18n("preferences.color_theme", "Dark")
light_theme = i18n("preferences.color_theme", "Light")


class ColorThemePage(QtWidgets.QWidget):
    COLOR_THEME_KEY = "ui.color_theme"

    def __init__(self, parent=None):
        super(ColorThemePage, self).__init__(parent=parent)
        self.setObjectName(i18n("preferences.color_theme", "Color Theme"))
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        rollout = RolloutWidget(i18n("preferences.color_theme", "Color Theme"), expandable=False)
        rollout_layout = QtWidgets.QFormLayout()
        rollout.set_layout(rollout_layout)
        layout.addWidget(rollout)
        layout.addStretch()
        self.setLayout(layout)

        self._color_theme = QtWidgets.QComboBox()
        self._color_theme.addItem(dark_theme)
        self._color_theme.addItem(light_theme)
        rollout_layout.addRow(i18n("preferences.color_theme", "Color Theme:"), self._color_theme)
        self.restore_previous_settings()

        self._warning = QtWidgets.QFrame()
        self._warning.setObjectName("color_theme_warning")
        self._warning.setStyleSheet(
            """
QFrame #color_theme_warning {
    background-color: rgba(255, 50, 50, 50);
    border-radius: 2px;
    border: 1px solid;
    border-color: rgba(255, 0, 0, 50);
}
"""
        )
        warning_layout = QtWidgets.QHBoxLayout()
        warning_label = QtWidgets.QLabel()
        warning_label.setPixmap(QtGui.QPixmap(":/icons/warning"))
        warning_layout.addWidget(warning_label)
        warning_layout.addWidget(
            QtWidgets.QLabel(
                i18n(
                    "preferences.color_theme",
                    "You must restart the program for the changes to take effect.",
                )
            )
        )
        warning_layout.addStretch()
        self._warning.setLayout(warning_layout)
        rollout_layout.addWidget(self._warning)
        self._warning.setHidden(True)

        self._color_theme.currentTextChanged.connect(self._color_theme_changed)

    def _color_theme_changed(self, control_key):
        self._warning.setHidden(False)

    def save_settings(self):
        settings = dcc_core.Application.instance().get_settings()
        color_theme = self._color_theme.currentText()
        if color_theme == dark_theme:
            value = "dark"
        else:
            value = "light"
        settings.set_string(self.COLOR_THEME_KEY, value)

    def restore_previous_settings(self):
        settings = dcc_core.Application.instance().get_settings()
        config = dcc_core.Application.instance().get_app_config()
        default_ui_theme = config.get_string("settings." + self.COLOR_THEME_KEY, "dark")
        value = settings.get_string(self.COLOR_THEME_KEY, default_ui_theme)
        if value == "dark":
            self._color_theme.setCurrentText(dark_theme)
        else:
            self._color_theme.setCurrentText(light_theme)
