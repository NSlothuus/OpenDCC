# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.core as dcc_core
from PySide6 import QtWidgets, QtCore, QtGui
import os
from opendcc.usd_ipc import start_usd_ipc_process
import opendcc.rendersystem as render_system
import subprocess

from opendcc.i18n import i18n


def open_render_view():
    app_root = dcc_core.Application.instance().get_application_root_path()
    RENDER_VIEW_PATH = os.path.join(app_root, "bin", "render_view")
    subprocess.Popen(RENDER_VIEW_PATH)


def start_ipr():
    if dcc_core.Application.instance().get_settings().get_bool("image_view.standalone", True):
        open_render_view()

    stop_render()
    render_system.RenderSystem.instance().init_render(render_system.RenderMethod.IPR)
    rc = render_system.RenderSystem.instance().render_control()
    if rc and rc.control_type() == "USD":  # TODO or HydraOp
        start_usd_ipc_process()
    render_system.RenderSystem.instance().start_render()


def stop_render():
    status = render_system.RenderSystem.instance().get_render_status()
    if (
        status == render_system.RenderStatus.IN_PROGRESS
        or status == render_system.RenderStatus.RENDERING
    ):
        render_system.RenderSystem.instance().stop_render()
    return


def preview_render():
    if dcc_core.Application.instance().get_settings().get_bool("image_view.standalone", True):
        open_render_view()

    stop_render()
    render_system.RenderSystem.instance().init_render(render_system.RenderMethod.PREVIEW)
    render_system.RenderSystem.instance().start_render()
    return


def disk_render():
    stop_render()

    app = dcc_core.Application.instance()
    settings = app.get_settings()
    setting_name = "render.disk.mode"
    mode = settings.get_string(setting_name, "current_frame")

    stage = app.get_session().get_current_stage()
    if not stage:
        return

    if mode == "stage_range":
        start = stage.GetStartTimeCode()
        end = stage.GetEndTimeCode()
        time_range = "{}:{}".format(start, end)
    elif mode == "custom_range":
        time_range = settings.get_string("render.disk.time_range", str(app.get_current_time()))
    else:
        time_range = str(app.get_current_time())

    render_system.RenderSystem.instance().render_control().set_attributes(
        {"time_range": time_range}
    )
    render_system.RenderSystem.instance().init_render(render_system.RenderMethod.DISK)
    render_system.RenderSystem.instance().start_render()


class OpenRenderViewAction(QtWidgets.QAction):
    def __init__(self, parent=None):
        QtWidgets.QAction.__init__(self, i18n("render_toolbar", "Open RenderView"), parent=parent)
        self.setIcon(QtGui.QIcon(":icons/open_render_view.png"))
        self.setToolTip(i18n("render_toolbar", "Open RenderView"))
        self.triggered.connect(open_render_view)


class PreviewRenderAction(QtWidgets.QAction):
    def __init__(self, parent=None):
        QtWidgets.QAction.__init__(self, i18n("render_toolbar", "Preview Render"), parent=parent)
        self.setIcon(QtGui.QIcon(":icons/render.png"))
        self.setToolTip(i18n("render_toolbar", "Preview Render"))
        self.triggered.connect(preview_render)


class IprRenderAction(QtWidgets.QAction):
    def __init__(self, parent=None):
        QtWidgets.QAction.__init__(self, i18n("render_toolbar", "Ipr Render"), parent=parent)
        self.setIcon(QtGui.QIcon(":icons/ipr.png"))
        self.setToolTip(i18n("render_toolbar", "Ipr Render"))
        self.triggered.connect(start_ipr)


class StopRenderAction(QtWidgets.QAction):
    def __init__(self, parent=None):
        QtWidgets.QAction.__init__(self, i18n("render_toolbar", "Stop Render"), parent=parent)
        self.setIcon(QtGui.QIcon(":icons/ipr_stop.png"))
        self.setToolTip(i18n("render_toolbar", "Stop Render"))
        self.triggered.connect(stop_render)

class DiskRenderAction(QtWidgets.QAction):
    def __init__(self, parent=None):
        QtWidgets.QAction.__init__(self, i18n("render_toolbar", "Disk Render"), parent=parent)
        self.setIcon(QtGui.QIcon(":icons/render_playblast.png"))
        self.setToolTip(i18n("render_toolbar", "Disk Render"))
        self.triggered.connect(disk_render)


def init_render_toolbar():
    main_win = dcc_core.Application.instance().get_main_window()
    render_toolbar = QtWidgets.QToolBar(i18n("render_toolbar", "Render"))
    render_toolbar.setObjectName("render_toolbar")
    render_toolbar.setProperty("opendcc_toolbar", True)
    render_toolbar.setProperty("opendcc_toolbar_side", "top")
    render_toolbar.setProperty("opendcc_toolbar_row", 0)
    render_toolbar.setProperty("opendcc_toolbar_index", 1)

    render_toolbar.setIconSize(QtCore.QSize(20, 20))

    dcc_core.Application.instance().register_event_callback("ui_escape_key_action", stop_render)

    open_render_view_action = OpenRenderViewAction(main_win)
    render_toolbar.addAction(open_render_view_action)

    preview_render_action = PreviewRenderAction(main_win)
    render_toolbar.addAction(preview_render_action)

    ipr_start_render_action = IprRenderAction(main_win)
    render_toolbar.addAction(ipr_start_render_action)

    stop_render_action = StopRenderAction(main_win)
    render_toolbar.addAction(stop_render_action)

    disk_render_action = DiskRenderAction(main_win)
    render_toolbar.addAction(disk_render_action)


    main_win.addToolBar(QtCore.Qt.TopToolBarArea, render_toolbar)
