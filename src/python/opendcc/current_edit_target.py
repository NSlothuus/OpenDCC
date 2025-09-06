# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.core as dcc_core
from PySide6 import QtWidgets, QtSvg, QtCore
from pxr import Usd, Tf, Sdf

from opendcc.i18n import i18n


class EditTargetPanel(QtWidgets.QWidget):
    def __init__(self, parent=None):
        QtWidgets.QWidget.__init__(self, parent)
        self._edit_target_listener = None
        self._edit_target_dirty_listener = None
        self._stage_changed_callback_id = None
        self._stage = None

        self.setToolTip(i18n("edit_target_panel", "Edit Target"))
        self.setLayout(QtWidgets.QHBoxLayout(self))
        label_icon = QtSvg.QSvgWidget(":/icons/svg/layers")
        label_icon.setFixedSize(QtCore.QSize(20, 20))
        self.layout().addWidget(label_icon)
        self.current_edit_target_label = QtWidgets.QLabel(self)
        self.layout().addWidget(self.current_edit_target_label)
        self.app = dcc_core.Application.instance()
        self.set_edit_target("")

    def set_edit_target(self, name):
        result_name = name if name else i18n("edit_target_panel", "None")
        self.current_edit_target_label.setText(result_name)

    def showEvent(self, event):
        """Add callbacks to keep ui in sync while visible"""
        self._stage_changed_callback_id = self.app.register_event_callback(
            "current_stage_changed", self.update_edit_target_display
        )
        self._edit_target_changed_callback_id = self.app.register_event_callback(
            "edit_target_changed", self.update_edit_target_display
        )
        self._edit_target_dirtiness_changed_callback_id = self.app.register_event_callback(
            "edit_target_dirtiness_changed", self.update_edit_target_display
        )

        # make sure editor starts in correct state
        self.update_edit_target_display()
        QtWidgets.QWidget.showEvent(self, event)

    def update_edit_target_display(self):
        session = self.app.get_session()
        self._stage = session.get_current_stage()
        if not self._stage:
            self.set_edit_target("")
        else:
            edit_target_layer = self._stage.GetEditTarget().GetLayer()
            edit_target_label = edit_target_layer.GetDisplayName()
            edit_target_label = (
                edit_target_label + "*" if edit_target_layer.dirty else edit_target_label
            )
            self.set_edit_target(edit_target_label)

    def hideEvent(self, event):
        """Remove callbacks on ui hide"""
        if self._stage_changed_callback_id:
            self.app.unregister_event_callback(
                "current_stage_changed", self._stage_changed_callback_id
            )
            self._stage_changed_callback_id = None
        if self._edit_target_changed_callback_id:
            self.app.unregister_event_callback(
                "edit_target_changed", self._edit_target_changed_callback_id
            )
            self._edit_target_changed_callback_id = None
        if self._edit_target_dirtiness_changed_callback_id:
            self.app.unregister_event_callback(
                "edit_target_dirtiness_changed",
                self._edit_target_dirtiness_changed_callback_id,
            )
            self._edit_target_dirtiness_changed_callback_id = None

        QtWidgets.QWidget.hideEvent(self, event)
