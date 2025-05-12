/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <usd_fallback_proxy/core/property_factory.h>
#include <usd_fallback_proxy/core/usd_property_proxy.h>

OPENDCC_NAMESPACE_OPEN

class FALLBACK_PROXY_API SourceRegistry
{
public:
    ~SourceRegistry() = default;
    static SourceRegistry& get_instance();

    static UsdPropertyProxyVector get_property_proxies(const PXR_NS::UsdPrim& prim);
    static UsdPropertyProxyPtr get_property_proxy(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& property_name);
    static bool is_prim_proxy_outdated(const PXR_NS::UsdPrim& prim, const PXR_NS::TfTokenVector& resynced_property_names,
                                       const PXR_NS::TfTokenVector& changed_property_names);
    static bool register_source(PropertyFactoryPtr property_factory);
    void load_plugins() const;

private:
    SourceRegistry();
    SourceRegistry(const SourceRegistry&) = delete;

    SourceRegistry& operator=(const SourceRegistry&) = delete;
    struct PropertyFactoryEntry
    {
        using This = PropertyFactoryEntry;
        PropertyFactoryPtr m_factory;
        uint64_t m_priority = 99;

        PropertyFactoryEntry(PropertyFactoryPtr factory);
        PropertyFactoryEntry(const PropertyFactoryEntry&) = delete;
        PropertyFactory* get_factory() const;

        struct LessThan
        {
            bool operator()(const This& left, const This& right) const;
        };
    };
    std::set<PropertyFactoryEntry, PropertyFactoryEntry::LessThan> m_sources;
};

OPENDCC_NAMESPACE_CLOSE
