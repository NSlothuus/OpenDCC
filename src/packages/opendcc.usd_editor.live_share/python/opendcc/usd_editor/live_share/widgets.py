# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtCore, QtGui
import opendcc.core as dcc_core
from opendcc.usd_ipc import start_usd_ipc_process

from opendcc.i18n import i18n


def init_live_share_toolbar():
    main_win = dcc_core.Application.instance().get_main_window()
    live_share_toolbar = QtWidgets.QToolBar(i18n("live_share", "Live Share"))
    live_share_toolbar.setObjectName("live_share")
    live_share_toolbar.setProperty("opendcc_toolbar", True)
    live_share_toolbar.setProperty("opendcc_toolbar_side", "top")
    live_share_toolbar.setProperty("opendcc_toolbar_row", 0)
    live_share_toolbar.setProperty("opendcc_toolbar_index", 4)
    live_share_toolbar.setIconSize(QtCore.QSize(20, 20))
    live_share_toolbar.setToolButtonStyle(QtCore.Qt.ToolButtonTextBesideIcon)
    enable_live_share_action = QtWidgets.QAction(
        QtGui.QIcon(":/icons/live_share"),
        i18n("live_share", "Enable Live Share"),
        live_share_toolbar,
    )
    live_share_toolbar.addAction(enable_live_share_action)
    enable_live_share_action.setCheckable(True)
    if dcc_core.Application.instance().get_session().is_live_sharing_enabled():
        enable_live_share_action.setChecked(True)

    def __on_checked_action(value):
        # TODO we should kill local process on disable live sharing
        host_name = (
            dcc_core.Application.instance()
            .get_settings()
            .get_string("live_share.hostname", "127.0.0.1")
        )
        if host_name == "127.0.0.1" or host_name.lower() == "localhost":
            start_usd_ipc_process()

        dcc_core.Application.instance().get_session().enable_live_sharing(value)

    enable_live_share_action.toggled.connect(__on_checked_action)
    main_win.addToolBar(QtCore.Qt.TopToolBarArea, live_share_toolbar)
