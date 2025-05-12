// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <algorithm>
#include <pxr/base/plug/plugin.h>
#include <pxr/base/plug/registry.h>
#include <usd_fallback_proxy/core/usd_prim_property_factory.h>
#include <usd_fallback_proxy/core/source_registry.h>
#include <usd_fallback_proxy/core/property_gatherer.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<SourceRegistry>();
    TfType::Define<PropertyFactory>();
}

struct UsdAttributeProxyLessThan
{
    bool operator()(const UsdPropertyProxyPtr& lhs, const UsdPropertyProxyPtr& rhs) const
    {
        return TfDictionaryLessThan()(lhs->get_name_token(), rhs->get_name_token());
    }
};

SourceRegistry::SourceRegistry()
{
    m_sources.emplace(std::make_unique<UsdPrimPropertyFactory>());
}

SourceRegistry& SourceRegistry::get_instance()
{
    static SourceRegistry instance;
    return instance;
}

UsdPropertyProxyVector SourceRegistry::get_property_proxies(const UsdPrim& prim)
{
    if (!prim)
        return {};

    UsdPropertyProxyVector result;
    auto& instance = get_instance();

    PropertyGatherer property_gatherer;

    for (const auto& source : instance.m_sources)
    {
        source.get_factory()->get_properties(prim, property_gatherer);
        if (!property_gatherer.m_current_properties.empty())
        {
            auto& cur_attributes = property_gatherer.m_current_properties;
            result.reserve(result.size() + cur_attributes.size());
            std::move(cur_attributes.begin(), cur_attributes.end(), std::back_inserter(result));
            property_gatherer.m_current_properties.clear();
        }
    }
    for (const auto& authored_prop : prim.GetAuthoredProperties())
    {
        property_gatherer.try_insert_property(authored_prop.Is<UsdAttribute>() ? SdfSpecType::SdfSpecTypeAttribute
                                                                               : SdfSpecType::SdfSpecTypeRelationship,
                                              authored_prop.GetName(), prim, UsdMetadataValueMap());
    }

    return property_gatherer.m_all_properties; // result;
}

UsdPropertyProxyPtr SourceRegistry::get_property_proxy(const UsdPrim& prim, const TfToken& property_name)
{
    if (!prim)
        return {};

    UsdPropertyProxyVector result;
    auto& instance = get_instance();

    PropertyGatherer property_gatherer;
    auto property = prim.GetProperty(property_name);
    if (property.IsAuthored())
    {
        property_gatherer.try_insert_property(property.Is<UsdAttribute>() ? SdfSpecType::SdfSpecTypeAttribute : SdfSpecType::SdfSpecTypeRelationship,
                                              property_name, prim, UsdMetadataValueMap());
    }

    for (const auto& source : instance.m_sources)
    {
        source.get_factory()->get_property(prim, property_name, property_gatherer);
    }

    return property_gatherer.m_all_properties.empty() ? nullptr : property_gatherer.m_all_properties.front();
}

bool SourceRegistry::is_prim_proxy_outdated(const UsdPrim& prim, const TfTokenVector& resynced_property_names,
                                            const TfTokenVector& changed_property_names)
{
    auto& instance = get_instance();
    for (const auto& source : instance.m_sources)
    {
        if (source.get_factory()->is_prim_proxy_outdated(prim, resynced_property_names, changed_property_names))
            return true;
    }

    return false;
}

bool SourceRegistry::register_source(PropertyFactoryPtr property_factory)
{
    if (TF_VERIFY(property_factory, "Attempt to register null property factory."))
    {
        get_instance().m_sources.emplace(std::move(property_factory));
        return true;
    }
    return false;
}

void SourceRegistry::load_plugins() const
{
    std::set<TfType> derived_plugins;
    PlugRegistry::GetInstance().GetAllDerivedTypes(TfType::Find<PropertyFactory>(), &derived_plugins);
    for (const auto& plugin_type : derived_plugins)
    {
        const auto plug = PlugRegistry::GetInstance().GetPluginForType(plugin_type);
        if (plug != nullptr)
        {
            if (plug->GetName() != "usd_fallback_proxy")
                if (!plug->Load())
                {
                    TF_RUNTIME_ERROR("Failed to load fallback proxy plugin \"%s\".", plug->GetName().c_str());
                }
        }
        else
        {
            TF_RUNTIME_ERROR("Failed to get plugin for extension type \"%s\".", plugin_type.GetTypeName().c_str());
        }
    }
}

SourceRegistry::PropertyFactoryEntry::PropertyFactoryEntry(PropertyFactoryPtr factory)
    : m_factory(std::move(factory))
{
    auto priority_value = PlugRegistry::GetInstance().GetDataFromPluginMetaData(m_factory->get_type(), "priority");
    if (priority_value.IsNull())
        return;
    m_priority = priority_value.GetUInt64();
}

PropertyFactory* SourceRegistry::PropertyFactoryEntry::get_factory() const
{
    return m_factory.get();
}

bool SourceRegistry::PropertyFactoryEntry::LessThan::operator()(const This& left, const This& right) const
{
    if (left.m_priority == right.m_priority)
        return left.m_factory.get() < right.m_factory.get();
    return left.m_priority < right.m_priority;
}
OPENDCC_NAMESPACE_CLOSE
