# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from . import _usdHydraOp
from pxr import Tf

Tf.PrepareModule(_usdHydraOp, locals())
del _usdHydraOp, Tf
try:
    from . import __DOC

    __DOC.Execute(locals())
    del __DOC
except Exception:
    pass
