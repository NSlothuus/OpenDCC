# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import sys, os
import traceback
import six


def execute_user_setup():
    """
    Look for dcc_user_setup.py in the search path and execute it in the "__main__"
    namespace
    """
    try:
        for path in sys.path[:]:
            script_path = os.path.join(path, "dcc_user_setup.py")
            if os.path.isfile(script_path):
                import __main__

                if six.PY2:
                    execfile(script_path, __main__.__dict__)
                else:
                    with open(script_path, "rb") as file:
                        source = file.read()
                    code = compile(source, script_path, "exec")
                    exec(code, __main__.__dict__)
    except Exception:
        try:
            etype, value, tb = sys.exc_info()
            tbStack = traceback.extract_tb(tb)
        finally:
            del tb
        sys.stderr.write("Failed to execute dcc_user_setup.py\n")
        sys.stderr.write("Traceback (most recent call last):\n")
        result = traceback.format_list(tbStack[1:]) + traceback.format_exception_only(etype, value)
        sys.stderr.write("".join(result))
