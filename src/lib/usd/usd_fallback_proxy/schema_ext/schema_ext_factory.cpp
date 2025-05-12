// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "schema_ext_factory.h"
#include <usd_fallback_proxy/core/source_registry.h>
#include <usd_fallback_proxy/core/property_gatherer.h>
#include "pxr/base/plug/registry.h"
#include "pxr/base/plug/plugin.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<SchemaExtFactory, TfType::Bases<PropertyFactory>>();
    SourceRegistry::register_source(std::make_unique<SchemaExtFactory>());
}

void SchemaExtFactory::get_properties(const UsdPrim& prim, PropertyGatherer& property_gatherer)
{
    if (!prim)
        return;
    if (!m_stage)
        initialize_stage();

    auto extended_prim = m_stage->GetPrimAtPath(SdfPath::AbsoluteRootPath().AppendChild(prim.GetTypeName()));
    if (!extended_prim)
        return;

    for (const auto& property : extended_prim.GetProperties())
    {
        property_gatherer.try_insert_property(property.Is<UsdAttribute>() ? SdfSpecType::SdfSpecTypeAttribute : SdfSpecType::SdfSpecTypeRelationship,
                                              property.GetName(), prim, property.GetAllMetadata(),
                                              UsdPropertySource(TfToken(), TfType::Find<SchemaExtFactory>()));
    }
}

void SchemaExtFactory::get_property(const UsdPrim& prim, const TfToken& property_name, PropertyGatherer& property_gatherer)
{
    if (!prim)
        return;
    if (!m_stage)
        initialize_stage();

    auto extended_prim = m_stage->GetPrimAtPath(SdfPath::AbsoluteRootPath().AppendChild(prim.GetTypeName()));
    if (!extended_prim)
        return;

    auto extended_property = extended_prim.GetProperty(property_name);
    if (!extended_property)
        return;

    property_gatherer.try_insert_property(
        extended_property.Is<UsdAttribute>() ? SdfSpecType::SdfSpecTypeAttribute : SdfSpecType::SdfSpecTypeRelationship, extended_property.GetName(),
        prim, extended_property.GetAllMetadata(), UsdPropertySource(TfToken(), TfType::Find<SchemaExtFactory>()));
}

bool SchemaExtFactory::is_prim_proxy_outdated(const UsdPrim& prim, const TfTokenVector& resynced_property_names,
                                              const TfTokenVector& changed_property_names) const
{
    return false;
}

TfType SchemaExtFactory::get_type() const
{
    return TfType::Find<SchemaExtFactory>();
}

void SchemaExtFactory::initialize_stage()
{
    std::set<TfType> extensions;
    PlugRegistry::GetInstance().GetAllDerivedTypes(get_type(), &extensions);
    m_stage = UsdStage::CreateInMemory();
    for (const auto& ext : extensions)
    {
        auto plugin = PlugRegistry::GetInstance().GetPluginForType(ext);
        if (plugin)
            m_stage->GetRootLayer()->InsertSubLayerPath(plugin->GetResourcePath() + "/schema_ext.usda");
    }
}
OPENDCC_NAMESPACE_CLOSE
