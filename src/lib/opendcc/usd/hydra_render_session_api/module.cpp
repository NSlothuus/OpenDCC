// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "pxr/pxr.h"
#include "pxr/base/tf/pyModule.h"
#include "hydra_render_session_api/renderSessionAPI.h"

PXR_NAMESPACE_USING_DIRECTIVE

TF_WRAP_MODULE
{
    TF_WRAP(UsdHydraExtRenderSessionAPI);
}
