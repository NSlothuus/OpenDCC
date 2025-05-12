# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtWidgets, QtCore, QtGui
from opendcc.actions.options_widget import make_option
from opendcc.common_widgets import RolloutWidget

from opendcc.i18n import i18n


class LiveSharePage(QtWidgets.QWidget):
    def __init__(self, parent=None):
        super(LiveSharePage, self).__init__(parent=parent)
        self.setObjectName("Live Share")
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        rollout = RolloutWidget(i18n("preferences.live_share", "Live Share"), expandable=False)
        rollout_layout = QtWidgets.QFormLayout()
        rollout.set_layout(rollout_layout)
        layout.addWidget(rollout)
        layout.addStretch()
        self.setLayout(layout)

        self.options = []

        self.options.append(
            make_option(
                i18n("preferences.live_share", "Host Name:"),
                "live_share.hostname",
                {"type": "string"},
                "127.0.0.1",
            )
        )

        self.options.append(
            make_option(
                i18n("preferences.live_share", "Listener Port:"),
                "live_share.listener_port",
                {"type": "int", "min": 0, "max": 65535},
                5561,
            )
        )

        self.options.append(
            make_option(
                i18n("preferences.live_share", "Publisher Port:"),
                "live_share.publisher_port",
                {"type": "int", "min": 0, "max": 65535},
                5562,
            )
        )

        self.options.append(
            make_option(
                i18n("preferences.live_share", "Sync Sender Port:"),
                "live_share.sync_sender_port",
                {"type": "int", "min": 0, "max": 65535},
                5560,
            )
        )

        self.options.append(
            make_option(
                i18n("preferences.live_share", "Sync Receiver Port:"),
                "live_share.sync_receiver_port",
                {"type": "int", "min": 0, "max": 65535},
                5559,
            )
        )

        for option in self.options:
            option.create_widget(rollout_layout, self)

        self.restore_previous_settings()

    def save_settings(self):
        for option in self.options:
            option.save_options()

    def restore_previous_settings(self):
        for option in self.options:
            option.load_options()
