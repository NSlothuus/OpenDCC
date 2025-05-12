# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import opendcc.dll_path_fix  # important workaround for windows starting from python-3.8
from . import _core
from pxr import Tf

Tf.PrepareModule(_core, locals())
del Tf

try:
    from . import __DOC

    __DOC.Execute(locals())
    del __DOC
except Exception:
    pass
