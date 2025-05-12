// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <usd_fallback_proxy/core/property_gatherer.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

bool PropertyGatherer::contains(const TfToken& property_name) const
{
    auto iter = std::find_if(m_all_properties.begin(), m_all_properties.end(),
                             [&property_name](const UsdPropertyProxyPtr& property) { return property->get_name_token() == property_name; });

    return iter != m_all_properties.end();
}

bool PropertyGatherer::try_insert_property(SdfSpecType spec_type, const TfToken& property_name, const UsdPrim& prim,
                                           const UsdMetadataValueMap& metadata, const UsdPropertySource& source, SdfPropertySpecHandle property_spec)
{
    auto attr_iter = std::find_if(m_all_properties.begin(), m_all_properties.end(),
                                  [&property_name](const UsdPropertyProxyPtr& property) { return property->get_name_token() == property_name; });
    if (attr_iter != m_all_properties.end())
    {
        auto& property = *attr_iter;
        for (auto metadata_entry : metadata)
        {
            auto& cur_val = property->m_metadata[metadata_entry.first];
            if (metadata_entry.second.IsHolding<VtDictionary>() && cur_val.IsHolding<VtDictionary>()) // merge dictionaries if possible
            {
                auto cur_dict = cur_val.UncheckedGet<VtDictionary>();
                for (const auto& val : metadata_entry.second.UncheckedGet<VtDictionary>())
                    cur_dict[val.first] = val.second;
                cur_val = VtValue(cur_dict);
            }
            else
            {
                cur_val = metadata_entry.second;
            }
        }
        if (!property->get_property_spec() && property_spec)
        {
            property->set_property_spec(property_spec);
        }
        property->append_source(source);

        return false;
    }
    std::vector<UsdPropertySource> sources;
    if (!source.get_source_plugin().IsUnknown())
        sources.push_back(source);
    auto new_property = std::make_shared<UsdPropertyProxy>(spec_type, prim, metadata, property_name, sources, property_spec);
    m_all_properties.push_back(new_property);
    m_current_properties.push_back(new_property);
    return true;
}

const PXR_NS::UsdMetadataValueMap& PropertyGatherer::get_metadata(const PXR_NS::TfToken& property_name) const
{
    static PXR_NS::UsdMetadataValueMap empty;
    auto prop_iter = std::find_if(m_all_properties.begin(), m_all_properties.end(),
                                  [&property_name](const auto& property) { return property->get_name_token() == property_name; });

    if (prop_iter == m_all_properties.end())
        return empty;

    return (*prop_iter)->m_metadata;
}

bool PropertyGatherer::update_metadata(const PXR_NS::TfToken& property_name, const PXR_NS::UsdMetadataValueMap& metadata)
{
    auto prop_iter = std::find_if(m_all_properties.begin(), m_all_properties.end(),
                                  [&property_name](const auto& property) { return property->get_name_token() == property_name; });

    if (prop_iter == m_all_properties.end())
        return false;

    auto& property = *prop_iter;
    for (auto metadata_entry : metadata)
    {
        auto& cur_val = property->m_metadata[metadata_entry.first];
        if (metadata_entry.second.IsHolding<VtDictionary>() && cur_val.IsHolding<VtDictionary>()) // merge dictionaries if possible
        {
            auto cur_dict = cur_val.UncheckedGet<VtDictionary>();
            for (const auto& val : metadata_entry.second.UncheckedGet<VtDictionary>())
                cur_dict[val.first] = val.second;
            cur_val = VtValue(cur_dict);
        }
        else
        {
            cur_val = metadata_entry.second;
        }
    }
    return true;
}
OPENDCC_NAMESPACE_CLOSE
