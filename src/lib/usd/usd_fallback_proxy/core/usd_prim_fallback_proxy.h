/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <usd_fallback_proxy/core/usd_fallback_proxy_watcher.h>
#include <usd_fallback_proxy/core/usd_property_proxy.h>

OPENDCC_NAMESPACE_OPEN

class FALLBACK_PROXY_API UsdPrimFallbackProxy
{
public:
    UsdPrimFallbackProxy() = default;
    explicit UsdPrimFallbackProxy(const PXR_NS::UsdPrim& prim);

    UsdPropertyProxyVector get_all_property_proxies() const;
    UsdPropertyProxyPtr get_property_proxy(const PXR_NS::TfToken& property_name) const;
    PXR_NS::UsdPrim get_usd_prim() const;

private:
    PXR_NS::UsdPrim m_prim;
    UsdFallbackProxyWatcher::PrimFallbackProxyChangedHandle m_handle;
};

OPENDCC_NAMESPACE_CLOSE