// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <pxr/base/tf/registryManager.h>
#include <pxr/base/tf/scriptModuleLoader.h>
PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfScriptModuleLoader)
{
    // List of direct dependencies for this library.
    const std::vector<TfToken> reqs = {};
    TfScriptModuleLoader::GetInstance().RegisterLibrary(TfToken("usd_fallback_proxy"), TfToken("opendcc.usd_fallback_proxy"), reqs);
}

PXR_NAMESPACE_CLOSE_SCOPE
