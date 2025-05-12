# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import os
import opendcc.core as dcc_core

try:
    SETTINGS_PATH = dcc_core.Application.instance().get_settings_path()
    if not SETTINGS_PATH:
        raise RuntimeError("Failed to get settings path for script editor")
    SETTINGS_PATH = "{0:s}/script_editor".format(SETTINGS_PATH)
except:
    SETTINGS_PATH = "{0}/.dcc/script_editor".format(os.path.dirname(__file__))
