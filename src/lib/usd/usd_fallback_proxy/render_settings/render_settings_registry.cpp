// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "usd/usd_fallback_proxy/render_settings/render_settings_registry.h"

#include <pxr/base/plug/registry.h>
#include <pxr/base/plug/plugin.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

RenderSettingsRegistry::RenderSettingsRegistry()
{
    initialize_stages();
}

/* static */
RenderSettingsRegistry& RenderSettingsRegistry::instance()
{
    static RenderSettingsRegistry registry;
    return registry;
}

UsdProperty RenderSettingsRegistry::get_property(const std::string& render_delegate, const UsdPrim& prim, const TfToken& property_name) const
{
    auto find = m_stages.find(render_delegate);

    if (!prim || find == m_stages.end())
    {
        return UsdProperty();
    }

    const auto& stage = find->second;

    const auto extended_prim = stage->GetPrimAtPath(SdfPath::AbsoluteRootPath().AppendChild(prim.GetTypeName()));
    if (!extended_prim)
    {
        return UsdProperty();
    }

    return extended_prim.GetProperty(property_name);
}

std::vector<UsdProperty> RenderSettingsRegistry::get_properties(const std::string& render_delegate, const UsdPrim& prim) const
{
    if (!prim || m_stages.find(render_delegate) == m_stages.end())
    {
        return {};
    }

    const auto& stage = m_stages.at(render_delegate);

    const auto extended_prim = stage->GetPrimAtPath(SdfPath::AbsoluteRootPath().AppendChild(prim.GetTypeName()));
    if (!extended_prim)
    {
        return {};
    }

    return extended_prim.GetProperties();
}

TfType RenderSettingsRegistry::get_type()
{
    return TfType::Find<RenderSettingsRegistry>();
}

void RenderSettingsRegistry::initialize_stages()
{
    const auto& plug_registry = PlugRegistry::GetInstance();

    std::set<TfType> extensions;
    plug_registry.GetAllDerivedTypes(get_type(), &extensions);

    for (const auto& ext : extensions)
    {
        auto plugin = plug_registry.GetPluginForType(ext);

        if (plugin)
        {
            auto& stage = m_stages[plugin->GetName()];
            if (!stage)
            {
                stage = UsdStage::CreateInMemory();
            }

            stage->GetRootLayer()->InsertSubLayerPath(plugin->GetResourcePath() + "/schema_ext.usda");
        }
    }
}

OPENDCC_NAMESPACE_CLOSE
