# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from . import _usdExpression
from pxr import Tf

Tf.PrepareModule(_usdExpression, locals())
del _usdExpression, Tf
try:
    from . import __DOC

    __DOC.Execute(locals())
    del __DOC
except Exception:
    pass
