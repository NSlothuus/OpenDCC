# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.core as dcc_core
import six


def update_window_title():
    app = dcc_core.Application.instance()
    session = app.get_session()
    main_window = app.get_main_window()
    config = app.get_app_config()

    default_title = six.u("{:s} {:s}-{:s}").format(
        config.get_string("settings.app.window.title"),
        config.get_string("settings.app.version"),
        app.get_commit_hash(),
    )
    stage = session.get_current_stage()
    if stage:
        layer = stage.GetEditTarget().GetLayer()
        layer_name = layer.identifier
        dirty_str = "*" if layer.dirty else ""
        main_window.setWindowTitle(
            six.u("{:s} -- {:s} {:s}").format(default_title, layer_name, dirty_str)
        )
        return
    main_window.setWindowTitle(default_title)


def init_update_window_title_callbacks():
    app = dcc_core.Application.instance()
    app.register_event_callback(
        dcc_core.Application.EventType.EDIT_TARGET_CHANGED, update_window_title
    )
    app.register_event_callback(
        dcc_core.Application.EventType.EDIT_TARGET_DIRTINESS_CHANGED, update_window_title
    )
    app.register_event_callback(
        dcc_core.Application.EventType.CURRENT_STAGE_CHANGED, update_window_title
    )
