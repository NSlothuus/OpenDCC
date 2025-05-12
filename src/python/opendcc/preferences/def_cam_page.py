# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtCore, QtGui
import sys
import opendcc.core as dcc_core
from opendcc.common_widgets import DoubleWidget, RolloutWidget

from opendcc.i18n import i18n


class DefCamPage(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super(DefCamPage, self).__init__(parent=parent)
        self.setObjectName(i18n("preferences.def_cam", "Default Camera"))
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        rollout = RolloutWidget(i18n("preferences.def_cam", "Default Camera"), expandable=False)
        rollout_layout = QtWidgets.QFormLayout()
        rollout.set_layout(rollout_layout)
        layout.addWidget(rollout)
        layout.addStretch()
        self.setLayout(layout)
        self.fov_ui = DoubleWidget()
        self.fov_ui.set_range(0, 180)
        self.fov_ui.enable_slider(5, 120)

        self.near_clip_plane_ui = DoubleWidget()
        self.near_clip_plane_ui.set_range(0.0001, sys.float_info.max)
        self.near_clip_plane_ui.setFixedWidth(80)
        self.far_clip_plane_ui = DoubleWidget()
        self.far_clip_plane_ui.set_range(0.0001, sys.float_info.max)
        self.far_clip_plane_ui.setFixedWidth(80)

        self.focal_length_ui = DoubleWidget()
        self.focal_length_ui.set_range(0.0001, sys.float_info.max)
        self.focal_length_ui.enable_slider(5, 250)

        self.horizontal_aperture_ui = DoubleWidget()
        self.horizontal_aperture_ui.set_range(0.0001, sys.float_info.max)
        self.horizontal_aperture_ui.setFixedWidth(80)

        self.vertical_aperture_ui = DoubleWidget()
        self.vertical_aperture_ui.set_range(0.0001, sys.float_info.max)
        self.vertical_aperture_ui.setFixedWidth(80)

        self.aspect_ratio_ui = DoubleWidget()
        self.aspect_ratio_ui.set_range(0.0001, sys.float_info.max)
        self.aspect_ratio_ui.enable_slider(0.0001, 10)

        self.clipping_range_ui = QtWidgets.QHBoxLayout()
        self.clipping_range_ui.addWidget(self.near_clip_plane_ui)
        self.clipping_range_ui.addWidget(self.far_clip_plane_ui)

        self.projection_ui = QtWidgets.QCheckBox()

        rollout_layout.addRow(i18n("preferences.def_cam", "Field of view:"), self.fov_ui)
        rollout_layout.addRow(
            i18n("preferences.def_cam", "Clipping plane:"), self.clipping_range_ui
        )
        rollout_layout.addRow(i18n("preferences.def_cam", "Focal length:"), self.focal_length_ui)
        rollout_layout.addRow(
            i18n("preferences.def_cam", "Horizontal aperture:"), self.horizontal_aperture_ui
        )
        rollout_layout.addRow(
            i18n("preferences.def_cam", "Vertical aperture:"), self.vertical_aperture_ui
        )
        rollout_layout.addRow(i18n("preferences.def_cam", "Aspect ratio:"), self.aspect_ratio_ui)
        rollout_layout.addRow(i18n("preferences.def_cam", "Perspective:"), self.projection_ui)
        self.load_values()
        self.save_values_to_restore()

        self.fov_ui.value_changed.connect(self.set_fov)
        self.near_clip_plane_ui.value_changed.connect(self.set_near_clip_plane)
        self.far_clip_plane_ui.value_changed.connect(self.set_far_clip_plane)
        self.focal_length_ui.value_changed.connect(self.set_focal_length)
        self.horizontal_aperture_ui.value_changed.connect(self.set_horizontal_aperture)
        self.vertical_aperture_ui.value_changed.connect(self.set_vertical_aperture)
        self.aspect_ratio_ui.value_changed.connect(self.set_aspect_ratio)
        self.projection_ui.toggled.connect(self.set_projection)

    def block_signals(self, value):
        self.fov_ui.blockSignals(value)
        self.near_clip_plane_ui.blockSignals(value)
        self.far_clip_plane_ui.blockSignals(value)
        self.focal_length_ui.blockSignals(value)
        self.horizontal_aperture_ui.blockSignals(value)
        self.vertical_aperture_ui.blockSignals(value)
        self.aspect_ratio_ui.blockSignals(value)
        self.projection_ui.blockSignals(value)

    def load_values(self):
        self.block_signals(True)
        self.fov_ui.set_value(dcc_core.DefCamSettings.instance().fov)
        self.near_clip_plane_ui.set_value(dcc_core.DefCamSettings.instance().near_clip_plane)
        self.far_clip_plane_ui.set_value(dcc_core.DefCamSettings.instance().far_clip_plane)
        self.focal_length_ui.set_value(dcc_core.DefCamSettings.instance().focal_length)
        self.horizontal_aperture_ui.set_value(
            dcc_core.DefCamSettings.instance().horizontal_aperture
        )
        self.vertical_aperture_ui.set_value(dcc_core.DefCamSettings.instance().vertical_aperture)
        self.aspect_ratio_ui.set_value(dcc_core.DefCamSettings.instance().aspect_ratio)
        self.projection_ui.setChecked(dcc_core.DefCamSettings.instance().is_perspective)
        self.block_signals(False)

    def save_values_to_restore(self):
        """Some crutch for saving state of setting when open preference window"""
        self.saved_fov_ui = dcc_core.DefCamSettings.instance().fov
        self.saved_near_clip_plane_ui = dcc_core.DefCamSettings.instance().near_clip_plane
        self.saved_far_clip_plane_ui = dcc_core.DefCamSettings.instance().far_clip_plane
        self.saved_focal_length_ui = dcc_core.DefCamSettings.instance().focal_length
        self.saved_horizontal_aperture_ui = dcc_core.DefCamSettings.instance().horizontal_aperture
        self.saved_vertical_aperture_ui = dcc_core.DefCamSettings.instance().vertical_aperture
        self.saved_aspect_ratio_ui = dcc_core.DefCamSettings.instance().aspect_ratio
        self.saved_projection_ui = dcc_core.DefCamSettings.instance().is_perspective

    def set_fov(self, value):
        dcc_core.DefCamSettings.instance().fov = value
        self.block_signals(True)
        self.focal_length_ui.set_value(dcc_core.DefCamSettings.instance().focal_length)
        self.vertical_aperture_ui.set_value(dcc_core.DefCamSettings.instance().vertical_aperture)
        self.horizontal_aperture_ui.set_value(
            dcc_core.DefCamSettings.instance().horizontal_aperture
        )
        self.block_signals(False)

    def set_near_clip_plane(self, value):
        dcc_core.DefCamSettings.instance().near_clip_plane = value

    def set_far_clip_plane(self, value):
        dcc_core.DefCamSettings.instance().far_clip_plane = value

    def set_focal_length(self, value):
        dcc_core.DefCamSettings.instance().focal_length = value
        self.block_signals(True)
        self.fov_ui.set_value(dcc_core.DefCamSettings.instance().fov)
        self.block_signals(False)

    def set_horizontal_aperture(self, value):
        dcc_core.DefCamSettings.instance().horizontal_aperture = value
        self.fov_ui.set_value(dcc_core.DefCamSettings.instance().fov)
        self.aspect_ratio_ui.set_value(dcc_core.DefCamSettings.instance().aspect_ratio)

    def set_vertical_aperture(self, value):
        dcc_core.DefCamSettings.instance().vertical_aperture = value
        self.block_signals(True)
        self.fov_ui.set_value(dcc_core.DefCamSettings.instance().fov)
        self.aspect_ratio_ui.set_value(dcc_core.DefCamSettings.instance().aspect_ratio)
        self.block_signals(False)

    def set_aspect_ratio(self, value):
        dcc_core.DefCamSettings.instance().aspect_ratio = value
        self.block_signals(True)
        self.horizontal_aperture_ui.set_value(
            dcc_core.DefCamSettings.instance().horizontal_aperture
        )
        self.fov_ui.set_value(dcc_core.DefCamSettings.instance().fov)
        self.block_signals(False)

    def set_projection(self, state):
        dcc_core.DefCamSettings.instance().is_perspective = self.projection_ui.isChecked()

    def save_settings(self):
        dcc_core.DefCamSettings.instance().save_settings()
        self.save_values_to_restore()

    def restore_previous_settings(self):
        self.set_fov(self.saved_fov_ui)
        self.set_near_clip_plane(self.saved_near_clip_plane_ui)
        self.set_far_clip_plane(self.saved_far_clip_plane_ui)
        self.set_focal_length(self.saved_focal_length_ui)
        self.set_horizontal_aperture(self.saved_horizontal_aperture_ui)
        self.set_vertical_aperture(self.saved_vertical_aperture_ui)
        self.set_aspect_ratio(self.saved_aspect_ratio_ui)
        self.set_projection(self.saved_projection_ui)
        self.load_values()
