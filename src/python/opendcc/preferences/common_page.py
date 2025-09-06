# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from PySide6 import QtWidgets, QtCore, QtGui
from opendcc.common_widgets import RolloutWidget
from opendcc.i18n import i18n
from opendcc.core import Application


class CommonPage(QtWidgets.QWidget):
    UI_LANGUAGE_KEY = "settings.ui.language"

    def __init__(self, parent=None):
        super(CommonPage, self).__init__(parent=parent)
        self.setObjectName(i18n("preferences.common", "Common"))
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        rollout = RolloutWidget(i18n("preferences.common", "Interface"), expandable=False)
        rollout_layout = QtWidgets.QFormLayout()
        rollout.set_layout(rollout_layout)
        layout.addWidget(rollout)
        layout.addStretch()
        self.setLayout(layout)

        supported_langs = Application.instance().get_supported_languages()
        default_lang = (
            Application.instance().get_app_config().get_string(self.UI_LANGUAGE_KEY, "en")
        )
        cur_lang = (
            Application.instance().get_settings().get_string(self.UI_LANGUAGE_KEY, default_lang)
        )

        self._lang_names_map = {}

        self._lang = QtWidgets.QComboBox()
        for l in supported_langs:
            lang_enum = QtCore.QLocale(l).language()
            beauty_name = QtCore.QLocale.languageToString(lang_enum)
            self._lang_names_map[beauty_name] = l
            self._lang.addItem(beauty_name)
            if l == cur_lang:
                self._lang.setCurrentText(beauty_name)

        rollout_layout.addRow(i18n("preferences.common.interface", "Language"), self._lang)
        self.restore_previous_settings()

        self._warning = QtWidgets.QFrame()
        self._warning.setObjectName("language_change_warning")
        self._warning.setStyleSheet(
            """
QFrame #language_change_warning {
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
                    "preferences.common",
                    "You must restart the program for the changes to take effect.",
                )
            )
        )
        warning_layout.addStretch()
        self._warning.setLayout(warning_layout)
        rollout_layout.addWidget(self._warning)
        self._warning.setHidden(True)

        self._lang.currentTextChanged.connect(self._lang_changed)

    def _lang_changed(self, text):
        lang_code = self._lang_names_map.get(text, "")
        if not lang_code:
            return
        Application.instance().get_settings().set_string(self.UI_LANGUAGE_KEY, lang_code)
        if Application.instance().set_ui_language(lang_code):
            self._warning.setHidden(False)

    def save_settings(self):
        settings = Application.instance().get_settings()
        current_language = self._lang.currentText()
        lang_code = self._lang_names_map[current_language]
        settings.set_string(self.UI_LANGUAGE_KEY, lang_code)

    def restore_previous_settings(self):
        settings = Application.instance().get_settings()
        config = Application.instance().get_app_config()
        default_lang = (
            Application.instance().get_app_config().get_string(self.UI_LANGUAGE_KEY, "en")
        )
        default_lang = config.get_string(self.UI_LANGUAGE_KEY, "en")
        cur_lang = settings.get_string(self.UI_LANGUAGE_KEY, default_lang)

        lang_enum = QtCore.QLocale(cur_lang).language()
        beauty_name = QtCore.QLocale.languageToString(lang_enum)
        self._lang.setCurrentText(beauty_name)
