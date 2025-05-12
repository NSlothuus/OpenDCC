// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <pxr/pxr.h>
#include <pxr/base/tf/registryManager.h>
#include <pxr/base/tf/scriptModuleLoader.h>
#include <pxr/base/tf/token.h>

#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfScriptModuleLoader)
{
    // List of direct dependencies for this library.
    const std::vector<TfToken> reqs = { TfToken("sdf"), TfToken("tf"), TfToken("usd") };
    TfScriptModuleLoader::GetInstance().RegisterLibrary(TfToken("usdHydraOp"), TfToken("UsdHydraOp"), reqs);
}

PXR_NAMESPACE_CLOSE_SCOPE
