# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import subprocess
import opendcc.core as dcc_core
import os
import sys

USD_IPC_PROCESS_NAME = "usd_ipc_broker"


def start_usd_ipc_process():
    process_start_path = os.path.join(
        dcc_core.Application.instance().get_application_root_path(),
        "bin",
        USD_IPC_PROCESS_NAME,
    )
    is_open = False
    if sys.platform == "win32":
        is_open = (
            str(
                subprocess.check_output(
                    ["tasklist", "/fi", "Imagename eq {}*".format(USD_IPC_PROCESS_NAME)]
                )
            ).find(USD_IPC_PROCESS_NAME)
            != -1
        )
    elif sys.platform.startswith("linux"):
        # https://stackoverflow.com/questions/55864216/unable-to-get-pid-of-a-process-by-name-in-python
        try:
            subprocess.check_output(["pidof", USD_IPC_PROCESS_NAME + ".bin"])
            is_open = True
        except subprocess.CalledProcessError:
            is_open = False
    elif sys.platform == "darwin":
        p = subprocess.Popen(["pgrep", USD_IPC_PROCESS_NAME])
        p.wait()
        is_open = not p.returncode

    if not is_open:
        subprocess.Popen(process_start_path)
