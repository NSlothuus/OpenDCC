// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "usd_fallback_proxy/render_settings/render_settings_property_factory.h"
#include "usd_fallback_proxy/core/source_registry.h"
#include "usd_fallback_proxy/core/property_gatherer.h"
#include "usd_fallback_proxy/render_settings/render_settings_registry.h"
#include "usd_fallback_proxy/utils/utils.h"

#include <pxr/usd/usdRender/settings.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>

#include "hydra_render_session_api/renderSessionAPI.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<RenderSettingsPropertyFactory, TfType::Bases<PropertyFactory>>();
    SourceRegistry::register_source(std::make_unique<RenderSettingsPropertyFactory>());
}

TF_DEFINE_PRIVATE_TOKENS(render_settings_tokens, (render_delegate));

void RenderSettingsPropertyFactory::get_property(const UsdPrim& prim, const TfToken& property_name, PropertyGatherer& property_gatherer)
{
    const auto settings = UsdHydraExtRenderSessionAPI(prim);
    if (settings && render_settings_tokens->render_delegate == property_name)
    {
        add_render_delegates(prim, property_gatherer);
    }

    auto usd_settings = UsdRenderSettings(prim);
    if (usd_settings)
    {
        const auto render_delegate = usd_fallback_proxy::utils::get_current_render_delegate_name(prim.GetStage());
        const auto property = RenderSettingsRegistry::instance().get_property(render_delegate, prim, property_name);
        if (property)
        {
            property_gatherer.try_insert_property(
                property.Is<UsdAttribute>() ? SdfSpecType::SdfSpecTypeAttribute : SdfSpecType::SdfSpecTypeRelationship, property.GetName(), prim,
                property.GetAllMetadata(), UsdPropertySource(TfToken(), TfType::Find<RenderSettingsPropertyFactory>()));
        }
    }
}

void RenderSettingsPropertyFactory::get_properties(const UsdPrim& prim, PropertyGatherer& property_gatherer)
{
    const auto settings = UsdHydraExtRenderSessionAPI(prim);
    if (settings)
    {
        add_render_delegates(prim, property_gatherer);
    }

    auto usd_settings = UsdRenderSettings(prim);
    if (usd_settings)
    {
        auto render_delegate = usd_fallback_proxy::utils::get_current_render_delegate_name(prim.GetStage());

        const auto properties = RenderSettingsRegistry::instance().get_properties(render_delegate, prim);

        for (const auto& property : properties)
        {
            if (property)
            {
                property_gatherer.try_insert_property(
                    property.Is<UsdAttribute>() ? SdfSpecType::SdfSpecTypeAttribute : SdfSpecType::SdfSpecTypeRelationship, property.GetName(), prim,
                    property.GetAllMetadata(), UsdPropertySource(TfToken(), TfType::Find<RenderSettingsPropertyFactory>()));
            }
        }
    }
}

bool RenderSettingsPropertyFactory::is_prim_proxy_outdated(const UsdPrim& prim, const TfTokenVector& resynced_property_names,
                                                           const TfTokenVector& changed_property_names) const
{
    return std::find(resynced_property_names.begin(), resynced_property_names.end(), render_settings_tokens->render_delegate) !=
               resynced_property_names.end() ||
           std::find(changed_property_names.begin(), changed_property_names.end(), render_settings_tokens->render_delegate) !=
               changed_property_names.end();
}

TfType RenderSettingsPropertyFactory::get_type() const
{
    return TfType::Find<RenderSettingsPropertyFactory>();
}

void RenderSettingsPropertyFactory::add_render_delegates(const UsdPrim& prim, PropertyGatherer& property_gatherer)
{
    PXR_NS::UsdMetadataValueMap metadata;
    PXR_NS::HfPluginDescVector plugins;
    PXR_NS::HdRendererPluginRegistry::GetInstance().GetPluginDescs(&plugins);

    VtArray<TfToken> plugin_tokens;
    plugin_tokens.reserve(plugins.size());

    for (const auto& plugin : plugins)
    {
        if (plugin.displayName == "GL")
        {
            plugin_tokens.push_back(PXR_NS::TfToken("Storm"));
        }
        else
        {
            plugin_tokens.push_back(PXR_NS::TfToken(plugin.displayName));
        }
    }

    metadata[SdfFieldKeys->AllowedTokens] = VtValue(plugin_tokens);
    metadata[SdfFieldKeys->TypeName] = VtValue(SdfValueTypeNames->Token.GetAsToken());
    metadata[SdfFieldKeys->Default] = VtValue(PXR_NS::TfToken("Storm"));

    property_gatherer.try_insert_property(SdfSpecType::SdfSpecTypeAttribute, render_settings_tokens->render_delegate, prim, metadata,
                                          UsdPropertySource(TfToken(), get_type()));
}

OPENDCC_NAMESPACE_CLOSE
