# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import UsdHdSession
from pxr import Tf

Tf.PrepareModule(UsdHdSession, locals())
del Tf

try:
    import __DOC

    __DOC.Execute(locals())
    del __DOC
except Exception:
    try:
        import __tmpDoc

        __tmpDoc.Execute(locals())
        del __tmpDoc
    except:
        pass
