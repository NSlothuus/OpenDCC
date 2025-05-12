# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtCore, QtGui, QtWidgets

from .file_menu import save_stage_if_needed

import opendcc.core as dcc_core


class AppCloseEventFilter(QtCore.QObject):
    def __init__(self, parent):
        QtCore.QObject.__init__(self, parent)

    def eventFilter(self, widget, event):
        if isinstance(widget, QtWidgets.QMainWindow):
            if event.type() == QtCore.QEvent.Close:
                app = dcc_core.Application.instance()
                session = app.get_session()
                for stage in session.get_stage_list():
                    if not save_stage_if_needed(stage):
                        event.ignore()
                        return True

        try:
            return QtCore.QObject.eventFilter(self, widget, event)
        except RuntimeError:
            return True
