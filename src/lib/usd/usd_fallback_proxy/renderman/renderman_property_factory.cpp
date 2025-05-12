// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <usd_fallback_proxy/renderman/renderman_property_factory.h>
#include <usd_fallback_proxy/core/source_registry.h>
#include <usd_fallback_proxy/core/property_gatherer.h>
#include <usd/usd_fallback_proxy/utils/utils.h>

#include <pxr/usd/usdShade/tokens.h>
#include <pxr/usd/usdShade/shader.h>
#include <pxr/usd/sdr/registry.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdRender/var.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<RendermanPropertyFactory, TfType::Bases<PropertyFactory>>();
    SourceRegistry::register_source(std::make_unique<RendermanPropertyFactory>());
}

TF_DEFINE_PRIVATE_TOKENS(ri_attribute_tokens, ((outputsOut, "outputs:out"))((outputsSurface, "outputs:ri:surface"))(
                                                  (outputsDisplacement, "outputs:ri:displacement"))((outputsVolume, "outputs:ri:volume")));

static const usd_fallback_proxy::utils::PropertyMap& get_renderman_properties()
{
    // driver:parameters:aov: from USD\third_party\renderman-24\plugin\hdPrman\renderParam.cpp:2742 branch dev

    static const usd_fallback_proxy::utils::PropertyMap properties = {
        // parameters from https://renderman.pixar.com/doxygen/rman24/classRiley.html#afc6061db7a909a3babaa7caab62a8586
        { // It's also used in USD\third_party\renderman-24\plugin\hdPrman\renderParam.cpp:2952 branch dev
          TfToken("driver:parameters:aov:remap"),
          { SdfSpecType::SdfSpecTypeAttribute,
            { { SdfFieldKeys->TypeName, VtValue(SdfValueTypeNames->Float3.GetAsToken()) }, { SdfFieldKeys->Default, VtValue(true) } } } },
        { TfToken("driver:parameters:aov:shadowthreshold"),
          { SdfSpecType::SdfSpecTypeAttribute,
            { { SdfFieldKeys->TypeName, VtValue(SdfValueTypeNames->Float.GetAsToken()) }, { SdfFieldKeys->Default, VtValue() } } } },
        // parameters from USD\third_party\renderman-24\plugin\hdPrman\renderParam.cpp:2881 branch dev
        { TfToken("driver:parameters:aov:rule"),
          { SdfSpecType::SdfSpecTypeAttribute,
            { { SdfFieldKeys->TypeName, VtValue(SdfValueTypeNames->String.GetAsToken()) }, { SdfFieldKeys->Default, VtValue() } } } },
        { TfToken("driver:parameters:aov:filter"),
          { SdfSpecType::SdfSpecTypeAttribute,
            { { SdfFieldKeys->TypeName, VtValue(SdfValueTypeNames->String.GetAsToken()) }, { SdfFieldKeys->Default, VtValue() } } } },
        { TfToken("driver:parameters:aov:filterwidth"),
          { SdfSpecType::SdfSpecTypeAttribute,
            { { SdfFieldKeys->TypeName, VtValue(SdfValueTypeNames->Float2.GetAsToken()) }, { SdfFieldKeys->Default, VtValue() } } } },
        { TfToken("driver:parameters:aov:statistics"),
          { SdfSpecType::SdfSpecTypeAttribute,
            { { SdfFieldKeys->TypeName, VtValue(SdfValueTypeNames->String.GetAsToken()) }, { SdfFieldKeys->Default, VtValue() } } } },
        { TfToken("driver:parameters:aov:relativepixelvariance"),
          { SdfSpecType::SdfSpecTypeAttribute,
            { { SdfFieldKeys->TypeName, VtValue(SdfValueTypeNames->Float.GetAsToken()) }, { SdfFieldKeys->Default, VtValue() } } } },
    };

    return properties;
}

static UsdMetadataValueMap get_outputs_metadata()
{
    static const UsdMetadataValueMap outputs_metadata = { { SdfFieldKeys->TypeName, VtValue(SdfValueTypeNames->Token.GetAsToken()) },
                                                          { SdfFieldKeys->Default, VtValue(TfToken()) } };
    return outputs_metadata;
}

static SdrShaderNodeConstPtr get_shader_node(const UsdPrim& prim)
{
    if (!prim)
        return nullptr;

    auto source_asset = prim.GetAttribute(UsdShadeTokens->infoId);
    TfToken shader_name;
    source_asset.Get(&shader_name);
    if (shader_name.IsEmpty())
        return nullptr;

    return SdrRegistry::GetInstance().GetShaderNodeByName(shader_name);
}

void RendermanPropertyFactory::get_properties(const UsdPrim& prim, PropertyGatherer& property_gatherer)
{
    const auto source = UsdPropertySource(TfToken(), TfType::Find<RendermanPropertyFactory>());

    if (auto var = UsdRenderVar(prim))
    {
        const auto stage = prim.GetStage();
        const std::string render_delegate = usd_fallback_proxy::utils::get_current_render_delegate_name(stage);

        if (render_delegate == "Prman" && usd_fallback_proxy::utils::is_connect_to_render_settings(var))
        {
            const auto& properties = get_renderman_properties();
            for (const auto& property : properties)
            {
                try_insert_property_pair(property, prim, property_gatherer, source);
            }
        }

        return;
    }

    if (UsdShadeMaterial(prim))
    {
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, ri_attribute_tokens->outputsSurface, prim, get_outputs_metadata(), source);
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, ri_attribute_tokens->outputsDisplacement, prim, get_outputs_metadata(), source);
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, ri_attribute_tokens->outputsVolume, prim, get_outputs_metadata(), source);
        return;
    }

    auto ri_shader = UsdShadeShader(prim);
    if (!ri_shader || ri_shader.GetImplementationSource() != UsdShadeTokens->id)
        return;

    auto sdr_node = get_shader_node(prim);
    if (!sdr_node)
        return;

    const auto outputs = sdr_node->GetOutputNames();
    if (outputs.empty())
    {
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, ri_attribute_tokens->outputsOut, prim, get_outputs_metadata(), source);
    }
}

void RendermanPropertyFactory::get_property(const UsdPrim& prim, const TfToken& property_name, PropertyGatherer& property_gatherer)
{
    const auto source = UsdPropertySource(TfToken(), TfType::Find<RendermanPropertyFactory>());

    if (auto var = UsdRenderVar(prim))
    {
        const auto stage = prim.GetStage();
        const std::string render_delegate = usd_fallback_proxy::utils::get_current_render_delegate_name(stage);

        if (render_delegate == "Prman" && usd_fallback_proxy::utils::is_connect_to_render_settings(var))
        {
            const auto& properties = get_renderman_properties();
            const auto find = properties.find(property_name);
            if (find != properties.end())
            {
                try_insert_property_pair(*find, prim, property_gatherer, source);
            }
        }

        return;
    }

    if (UsdShadeMaterial(prim))
    {
        if (property_name == ri_attribute_tokens->outputsSurface)
        {
            property_gatherer.try_insert_property(SdfSpecTypeAttribute, ri_attribute_tokens->outputsSurface, prim, get_outputs_metadata(), source);
        }
        else if (property_name == ri_attribute_tokens->outputsDisplacement)
        {
            property_gatherer.try_insert_property(SdfSpecTypeAttribute, ri_attribute_tokens->outputsDisplacement, prim, get_outputs_metadata(),
                                                  source);
        }
        else if (property_name == ri_attribute_tokens->outputsVolume)
        {
            property_gatherer.try_insert_property(SdfSpecTypeAttribute, ri_attribute_tokens->outputsVolume, prim, get_outputs_metadata(), source);
        }
        return;
    }

    auto ri_shader = UsdShadeShader(prim);
    if (!ri_shader || ri_shader.GetImplementationSource() != UsdShadeTokens->id)
        return;

    auto sdr_node = get_shader_node(prim);
    if (!sdr_node)
        return;

    if (property_name == ri_attribute_tokens->outputsOut && sdr_node->GetOutputNames().empty())
    {
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, ri_attribute_tokens->outputsOut, prim, get_outputs_metadata(), source);
    }
}

bool RendermanPropertyFactory::is_prim_proxy_outdated(const UsdPrim& prim, const TfTokenVector& resynced_property_names,
                                                      const TfTokenVector& changed_property_names) const
{
    return false;
}

TfType RendermanPropertyFactory::get_type() const
{
    return TfType::Find<RendermanPropertyFactory>();
}

OPENDCC_NAMESPACE_CLOSE
