// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <usd_fallback_proxy/core/usd_prim_fallback_proxy.h>
#include <usd_fallback_proxy/core/source_registry.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

UsdPrimFallbackProxy::UsdPrimFallbackProxy(const UsdPrim& prim)
    : m_prim(prim)
{
    m_handle = UsdFallbackProxyWatcher::register_prim_fallback_proxy(prim);
}

UsdPropertyProxyVector UsdPrimFallbackProxy::get_all_property_proxies() const
{
    return SourceRegistry::get_property_proxies(m_prim);
}

UsdPropertyProxyPtr UsdPrimFallbackProxy::get_property_proxy(const TfToken& property_name) const
{
    return SourceRegistry::get_property_proxy(m_prim, property_name);
}

UsdPrim UsdPrimFallbackProxy::get_usd_prim() const
{
    return m_prim;
}
OPENDCC_NAMESPACE_CLOSE
