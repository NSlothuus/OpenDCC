# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from __future__ import absolute_import
import opendcc.core as dcc_core
from PySide6 import QtWidgets, QtCore, QtGui
from opendcc.common_widgets import DoubleWidget, RolloutWidget, ColorButton
from opendcc.i18n import i18n


class ColorValueWidget(QtWidgets.QWidget):

    value_changed_signal = QtCore.Signal()

    def __init__(self, parent=None, with_alpha=False):
        QtWidgets.QWidget.__init__(self, parent)
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)
        self.button = ColorButton(self, with_alpha)
        layout.addWidget(self.button)
        self._with_alpha = with_alpha
        self.setFixedWidth(40)
        self.setFixedHeight(16)
        self.button.color_changed.connect(self.value_changed)

    def set_value(self, value):
        if value is None:
            return

        color = QtGui.QColor()
        color.setRgbF(*value)
        self.button.color(color)
        self.button.repaint()

    def value_changed(self, *args):
        self.value_changed_signal.emit()

    def get_value(self):
        r, g, b, a = self.button.color().getRgbF()
        if self._with_alpha:
            return r, g, b, a
        return r, g, b


class ViewportPage(QtWidgets.QWidget):
    _selection_color_key = "viewport.selection_color"
    _background_gradient_enable_key = "viewport.background.gradient_enable"
    _background_color_key = "viewport.background.color"
    _background_default_color_key = "settings.viewport.background.default_color"
    _background_gradient_top_key = "viewport.background.gradient_top"
    _background_gradient_bottom_key = "viewport.background.gradient_bottom"
    _global_scale_key = "viewport.manipulators.global_scale"
    _grid_enable_key = "viewport.grid.enable"
    _grid_lines_color_key = "viewport.grid.lines_color"
    _grid_min_step_key = "viewport.grid.min_step"
    _show_camera_key = "viewport.show_camera"
    _overlay_camera_key = "viewport.overlay.camera"
    _overlay_renderer_key = "viewport.overlay.renderer"
    _overlay_scene_context_key = "viewport.overlay.scene_context"
    _overlay_edit_target_key = "viewport.overlay.edit_target"
    _default_render_delegate_key = "viewport.default_render_delegate"

    def __init__(self, parent=None):
        super(ViewportPage, self).__init__(parent=parent)
        self.setObjectName(i18n("preferences.viewport", "Viewport"))
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        rollout = RolloutWidget(i18n("preferences.viewport", "Viewport"), expandable=False)
        rollout_layout = QtWidgets.QFormLayout()
        rollout.set_layout(rollout_layout)
        layout.addWidget(rollout)
        self.setLayout(layout)
        self._selection_color_widget = ColorValueWidget(with_alpha=True)
        self._enable_background_gradient_widget = QtWidgets.QCheckBox()
        self._background_color_widget = ColorValueWidget()
        self._gradient_top_color_widget = ColorValueWidget()
        self._gradient_bottom_color_widget = ColorValueWidget()
        self._global_scale_widget = DoubleWidget()
        self._global_scale_widget.set_range(0, 10)
        self._global_scale_widget.set_precision(2)
        self._global_scale_widget.enable_slider(0, 10)
        self._grid_enable_widget = QtWidgets.QCheckBox()
        self._grid_lines_color_widget = ColorValueWidget(with_alpha=True)
        self._grid_min_step_widget = DoubleWidget()
        self._grid_min_step_widget.set_range(0.0001, 10000.0)
        self._grid_min_step_widget.enable_slider(0.1, 5)
        default_render_delegate_layout = QtWidgets.QHBoxLayout()
        self._default_render_delegate_widget = QtWidgets.QComboBox()
        default_render_delegate_layout.addWidget(self._default_render_delegate_widget)
        default_render_delegate_layout.addStretch()
        self._default_render_delegate_widget.setEditable(False)
        available_render_delegates = []
        for plugin in dcc_core.ViewportView.get_render_plugins():
            name = dcc_core.ViewportView.get_render_display_name(plugin)
            if name == "GL":
                name = "Storm"
            available_render_delegates.append(name)
        self._default_render_delegate_widget.addItems(available_render_delegates)

        config = dcc_core.Application.get_app_config()
        settings = dcc_core.Application.instance().get_settings()
        self._old_selection_color_value = settings.get_double_array(
            self._selection_color_key, [1, 1, 0, 0.5]
        )
        self._old_background_gradient_enable = settings.get_bool(
            self._background_gradient_enable_key, False
        )
        self._old_background_color = settings.get_double_array(
            self._background_color_key,
            config.get_double_array(self._background_default_color_key, [0.3, 0.3, 0.3]),
        )
        self._old_background_gradient_top_color = settings.get_double_array(
            self._background_gradient_top_key, [1, 1, 1]
        )
        self._old_background_gradient_bottom_color = settings.get_double_array(
            self._background_gradient_bottom_key, [0, 0, 0]
        )
        self._old_global_scale_value = settings.get_double(self._global_scale_key, 1.0)
        self._old_grid_enable = settings.get_bool(self._grid_enable_key, True)
        self._old_grid_lines_color = settings.get_double_array(
            self._grid_lines_color_key, [0.56, 0.56, 0.56, 1]
        )
        self._old_grid_min_step = settings.get_double(self._grid_min_step_key, 1.0)
        self._old_default_render_delegate = settings.get_string(
            self._default_render_delegate_key, "Storm"
        )

        self._selection_color_widget.set_value(self._old_selection_color_value)
        self._enable_background_gradient_widget.setChecked(self._old_background_gradient_enable)
        self._background_color_widget.set_value(self._old_background_color)
        self._gradient_top_color_widget.set_value(self._old_background_gradient_top_color)
        self._gradient_bottom_color_widget.set_value(self._old_background_gradient_bottom_color)
        self._global_scale_widget.set_value(self._old_global_scale_value)
        self._grid_enable_widget.setChecked(self._old_grid_enable)
        self._grid_lines_color_widget.set_value(self._old_grid_lines_color)
        self._grid_min_step_widget.set_value(self._old_grid_min_step)
        self._default_render_delegate_widget.setCurrentText(self._old_default_render_delegate)

        rollout_layout.addRow(
            i18n("preferences.viewport", "Selection Color:"), self._selection_color_widget
        )
        rollout_layout.addRow(
            i18n("preferences.viewport", "Enable Background Gradient:"),
            self._enable_background_gradient_widget,
        )
        rollout_layout.addRow(
            i18n("preferences.viewport", "Background Color:"), self._background_color_widget
        )
        rollout_layout.addRow(
            i18n("preferences.viewport", "Gradient Top Color:"), self._gradient_top_color_widget
        )
        rollout_layout.addRow(
            i18n("preferences.viewport", "Gradient Bottom Color:"),
            self._gradient_bottom_color_widget,
        )
        rollout_layout.addRow(
            i18n("preferences.viewport", "Global Scale:"), self._global_scale_widget
        )
        rollout_layout.addRow(
            i18n("preferences.viewport", "Enable Grid:"), self._grid_enable_widget
        )
        rollout_layout.addRow(
            i18n("preferences.viewport", "Grid Lines Color:"), self._grid_lines_color_widget
        )
        rollout_layout.addRow(
            i18n("preferences.viewport", "Grid Minimum Step:"), self._grid_min_step_widget
        )
        rollout_layout.addRow(
            i18n("preferences.viewport", "Default Render Delegate:"), default_render_delegate_layout
        )

        overlay_rollout = RolloutWidget(i18n("preferences.viewport", "Overlay"), expandable=False)
        overlay_rollout_layout = QtWidgets.QFormLayout()
        overlay_rollout.set_layout(overlay_rollout_layout)
        layout.addWidget(overlay_rollout)
        layout.addStretch()

        self._old_show_camera = settings.get_bool(self._show_camera_key, True)
        self._show_camera_widget = QtWidgets.QCheckBox()
        self._show_camera_widget.setChecked(self._old_show_camera)
        self._show_camera_widget.toggled.connect(self._on_show_camera)
        overlay_rollout_layout.addRow(
            i18n("preferences.viewport", "Show Camera:"), self._show_camera_widget
        )

        self._old_overlay_camera = settings.get_bool(self._overlay_camera_key, True)
        self._old_overlay_renderer = settings.get_bool(self._overlay_renderer_key, True)
        self._old_overlay_scene_context = settings.get_bool(self._overlay_scene_context_key, False)
        self._old_overlay_edit_target = settings.get_bool(self._overlay_edit_target_key, True)

        self._overlay_camera_widget = QtWidgets.QCheckBox()
        self._overlay_renderer_widget = QtWidgets.QCheckBox()
        self._overlay_scene_context_widget = QtWidgets.QCheckBox()
        self._overlay_edit_target_widget = QtWidgets.QCheckBox()

        overlay_rollout_layout.addRow(
            i18n("preferences.viewport", "Camera Overlay:"), self._overlay_camera_widget
        )
        overlay_rollout_layout.addRow(
            i18n("preferences.viewport", "Renderer Overlay:"), self._overlay_renderer_widget
        )
        overlay_rollout_layout.addRow(
            i18n("preferences.viewport", "Scene Context Overlay:"),
            self._overlay_scene_context_widget,
        )
        overlay_rollout_layout.addRow(
            i18n("preferences.viewport", "Edit Target Overlay:"), self._overlay_edit_target_widget
        )

        self._overlay_camera_widget.setChecked(self._old_overlay_camera)
        self._overlay_renderer_widget.setChecked(self._old_overlay_renderer)
        self._overlay_scene_context_widget.setChecked(self._old_overlay_scene_context)
        self._overlay_edit_target_widget.setChecked(self._old_overlay_edit_target)

        self._overlay_camera_widget.toggled.connect(self._on_overlay_camera)
        self._overlay_renderer_widget.toggled.connect(self._on_overlay_renderer)
        self._overlay_scene_context_widget.toggled.connect(self._on_overlay_scene_context)
        self._overlay_edit_target_widget.toggled.connect(self._on_overlay_edit_target)

        self._selection_color_widget.value_changed_signal.connect(self._on_selection_color_changed)
        self._enable_background_gradient_widget.toggled.connect(self._on_background_gradient_enable)
        self._background_color_widget.value_changed_signal.connect(
            self._on_background_color_changed
        )
        self._gradient_top_color_widget.value_changed_signal.connect(
            self._on_gradient_top_color_changed
        )
        self._gradient_bottom_color_widget.value_changed_signal.connect(
            self._on_gradient_bottom_color_changed
        )
        self._global_scale_widget.value_changed.connect(self._on_global_scale_changed)
        self._grid_enable_widget.toggled.connect(self._on_grid_enable)
        self._grid_lines_color_widget.value_changed_signal.connect(
            self._on_grid_lines_color_changed
        )
        self._grid_min_step_widget.value_changed.connect(self._on_grid_min_step_changed)
        self._default_render_delegate_widget.currentTextChanged.connect(
            self._on_default_render_delegate_changed
        )

    def _on_show_camera(self, state):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_bool(self._show_camera_key, self._show_camera_widget.isChecked())

    def _on_overlay_camera(self, state):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_bool(self._overlay_camera_key, self._overlay_camera_widget.isChecked())

    def _on_overlay_renderer(self, state):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_bool(self._overlay_renderer_key, self._overlay_renderer_widget.isChecked())

    def _on_overlay_scene_context(self, state):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_bool(
            self._overlay_scene_context_key,
            self._overlay_scene_context_widget.isChecked(),
        )

    def _on_overlay_edit_target(self, state):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_bool(
            self._overlay_edit_target_key, self._overlay_edit_target_widget.isChecked()
        )

    def _on_background_color_changed(self):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_double_array(
            self._background_color_key, self._background_color_widget.get_value()
        )

    def _on_gradient_top_color_changed(self):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_double_array(
            self._background_gradient_top_key,
            self._gradient_top_color_widget.get_value(),
        )

    def _on_gradient_bottom_color_changed(self):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_double_array(
            self._background_gradient_bottom_key,
            self._gradient_bottom_color_widget.get_value(),
        )

    def _on_background_gradient_enable(self, state):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_bool(self._background_gradient_enable_key, state)

    def _on_selection_color_changed(self):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_double_array(
            self._selection_color_key, self._selection_color_widget.get_value()
        )

    def _on_global_scale_changed(self):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_double(self._global_scale_key, self._global_scale_widget.get_value())

    def _on_grid_enable(self, state):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_bool(self._grid_enable_key, self._grid_enable_widget.isChecked())

    def _on_grid_lines_color_changed(self):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_double_array(
            self._grid_lines_color_key, self._grid_lines_color_widget.get_value()
        )

    def _on_grid_min_step_changed(self):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_double(self._grid_min_step_key, self._grid_min_step_widget.get_value())

    def _on_default_render_delegate_changed(self, text):
        settings = dcc_core.Application.instance().get_settings()
        settings.set_string(self._default_render_delegate_key, text)

    def save_settings(self):
        current_selection_color = self._selection_color_widget.get_value()
        background_gradient_enable = self._enable_background_gradient_widget.isChecked()
        background_color = self._background_color_widget.get_value()
        background_gradient_top_color = self._gradient_top_color_widget.get_value()
        background_gradient_bottom_color = self._gradient_bottom_color_widget.get_value()
        global_scale_value = self._global_scale_widget.get_value()

        self._old_selection_color_value = current_selection_color
        self._old_background_gradient_enable = background_gradient_enable
        self._old_background_color = background_color
        self._old_background_gradient_top_color = background_gradient_top_color
        self._old_background_gradient_bottom_color = background_gradient_bottom_color
        self._old_global_scale_value = global_scale_value
        self._old_grid_enable = self._grid_enable_widget.isChecked()
        self._old_grid_lines_color = self._grid_lines_color_widget.get_value()
        self._old_grid_min_step = self._grid_min_step_widget.get_value()
        self._old_default_render_delegate = self._default_render_delegate_widget.currentText()

        self._old_show_camera = self._show_camera_widget.isChecked()

        self._old_overlay_camera = self._overlay_camera_widget.isChecked()
        self._old_overlay_renderer = self._overlay_renderer_widget.isChecked()
        self._old_overlay_scene_context = self._overlay_scene_context_widget.isChecked()
        self._old_overlay_edit_target = self._overlay_edit_target_widget.isChecked()

        self.restore_previous_settings()

    def restore_previous_settings(self):
        self._selection_color_widget.set_value(self._old_selection_color_value)
        settings = dcc_core.Application.instance().get_settings()
        settings.set_double_array(self._selection_color_key, self._old_selection_color_value)
        settings.set_bool(
            self._background_gradient_enable_key, self._old_background_gradient_enable
        )
        settings.set_double_array(self._background_color_key, self._old_background_color)
        settings.set_double_array(
            self._background_gradient_top_key, self._old_background_gradient_top_color
        )
        settings.set_double_array(
            self._background_gradient_bottom_key,
            self._old_background_gradient_bottom_color,
        )
        settings.set_double(self._global_scale_key, self._old_global_scale_value)
        settings.set_bool(self._grid_enable_key, self._old_grid_enable)
        settings.set_double_array(self._grid_lines_color_key, self._old_grid_lines_color)
        settings.set_double(self._grid_min_step_key, self._old_grid_min_step)
        settings.set_string(self._default_render_delegate_key, self._old_default_render_delegate)

        settings.set_bool(self._show_camera_key, self._show_camera_widget.isChecked())

        settings.set_bool(self._overlay_camera_key, self._overlay_camera_widget.isChecked())
        settings.set_bool(self._overlay_renderer_key, self._overlay_renderer_widget.isChecked())
        settings.set_bool(
            self._overlay_scene_context_key,
            self._overlay_scene_context_widget.isChecked(),
        )
        settings.set_bool(
            self._overlay_edit_target_key, self._overlay_edit_target_widget.isChecked()
        )
