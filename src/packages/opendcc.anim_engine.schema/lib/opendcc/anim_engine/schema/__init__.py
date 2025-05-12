# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from . import _usdAnimEngine
from pxr import Tf

Tf.PrepareModule(_usdAnimEngine, locals())
del _usdAnimEngine, Tf
try:
    from . import __DOC

    __DOC.Execute(locals())
    del __DOC
except Exception:
    pass
