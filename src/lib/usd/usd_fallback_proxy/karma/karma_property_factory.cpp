// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <usd_fallback_proxy/karma/karma_property_factory.h>
#include <usd_fallback_proxy/core/source_registry.h>
#include <usd_fallback_proxy/core/property_gatherer.h>
#include <pxr/usd/usdShade/tokens.h>
#include <pxr/usd/usdShade/shader.h>
#include <pxr/usd/sdr/registry.h>
#include <pxr/usd/usdShade/material.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<KarmaPropertyFactory, TfType::Bases<PropertyFactory>>();
    SourceRegistry::register_source(std::make_unique<KarmaPropertyFactory>());
}

TF_DEFINE_PRIVATE_TOKENS(karma_attribute_tokens, ((outputsOut, "outputs:out"))((outputsSurface, "outputs:karma:surface"))(
                                                     (outputsDisplacement, "outputs:karma:displacement"))((outputsVolume, "outputs:karma:volume")));

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

void KarmaPropertyFactory::get_properties(const UsdPrim& prim, PropertyGatherer& property_gatherer)
{
    const auto source = UsdPropertySource(TfToken(), get_type());
    if (UsdShadeMaterial(prim))
    {
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, karma_attribute_tokens->outputsSurface, prim, get_outputs_metadata(), source);
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, karma_attribute_tokens->outputsDisplacement, prim, get_outputs_metadata(),
                                              source);
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, karma_attribute_tokens->outputsVolume, prim, get_outputs_metadata(), source);
        return;
    }

    auto karma_shader = UsdShadeShader(prim);
    if (!karma_shader || karma_shader.GetImplementationSource() != UsdShadeTokens->id)
        return;

    auto sdr_node = get_shader_node(prim);
    if (!sdr_node)
        return;

    const auto outputs = sdr_node->GetOutputNames();
    if (outputs.empty())
    {
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, karma_attribute_tokens->outputsOut, prim, get_outputs_metadata(), source);
    }
}

void KarmaPropertyFactory::get_property(const UsdPrim& prim, const TfToken& property_name, PropertyGatherer& property_gatherer)
{
    const auto source = UsdPropertySource(TfToken(), get_type());
    if (UsdShadeMaterial(prim))
    {
        if (property_name == karma_attribute_tokens->outputsSurface)
        {
            property_gatherer.try_insert_property(SdfSpecTypeAttribute, karma_attribute_tokens->outputsSurface, prim, get_outputs_metadata(), source);
        }
        else if (property_name == karma_attribute_tokens->outputsDisplacement)
        {
            property_gatherer.try_insert_property(SdfSpecTypeAttribute, karma_attribute_tokens->outputsDisplacement, prim, get_outputs_metadata(),
                                                  source);
        }
        else if (property_name == karma_attribute_tokens->outputsVolume)
        {
            property_gatherer.try_insert_property(SdfSpecTypeAttribute, karma_attribute_tokens->outputsVolume, prim, get_outputs_metadata(), source);
        }
        return;
    }

    auto karma_shader = UsdShadeShader(prim);
    if (!karma_shader || karma_shader.GetImplementationSource() != UsdShadeTokens->id)
        return;

    auto sdr_node = get_shader_node(prim);
    if (!sdr_node)
        return;

    if (property_name == karma_attribute_tokens->outputsOut && sdr_node->GetOutputNames().empty())
    {
        property_gatherer.try_insert_property(SdfSpecTypeAttribute, karma_attribute_tokens->outputsOut, prim, get_outputs_metadata(),
                                              UsdPropertySource(TfToken(), get_type()));
    }
}

bool KarmaPropertyFactory::is_prim_proxy_outdated(const UsdPrim& prim, const TfTokenVector& resynced_property_names,
                                                  const TfTokenVector& changed_property_names) const
{
    return false;
}

TfType KarmaPropertyFactory::get_type() const
{
    return TfType::Find<KarmaPropertyFactory>();
}

OPENDCC_NAMESPACE_CLOSE
