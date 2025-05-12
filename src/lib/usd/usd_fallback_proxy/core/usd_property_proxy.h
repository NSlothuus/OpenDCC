/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <usd/usd_fallback_proxy/core/api.h>
#include <usd/usd_fallback_proxy/core/usd_property_source.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/property.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/relationship.h>
#include <pxr/usd/sdf/propertySpec.h>
#include <pxr/usd/usd/relationship.h>
#include <pxr/usd/sdf/attributeSpec.h>
#include <pxr/usd/sdf/relationshipSpec.h>
#include <pxr/usd/sdr/shaderProperty.h>

OPENDCC_NAMESPACE_OPEN

class PropertyGatherer;
class UsdPropertySource;

class FALLBACK_PROXY_API UsdPropertyProxy
{
public:
    UsdPropertyProxy() = default;

    explicit UsdPropertyProxy(PXR_NS::SdfSpecType type, const PXR_NS::UsdPrim& prim, const PXR_NS::UsdMetadataValueMap& metadata,
                              const PXR_NS::TfToken& name, const std::vector<UsdPropertySource>& sources,
                              const PXR_NS::SdfPropertySpecHandle& property_spec)
        : m_type(type)
        , m_prim(prim)
        , m_metadata(metadata)
        , m_name(name)
        , m_sources(sources)
        , m_property_spec(property_spec)
    {
    }

    bool get(PXR_NS::VtValue* value, PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default()) const;
    bool get_default(PXR_NS::VtValue* value) const;
    bool set(const PXR_NS::VtValue& value, PXR_NS::UsdTimeCode time = PXR_NS::UsdTimeCode::Default());
    void set_property_spec(PXR_NS::SdfPropertySpecHandle property_spec);
    PXR_NS::SdfPropertySpecHandle get_property_spec() const;
    PXR_NS::TfToken get_name_token() const;
    PXR_NS::SdfValueTypeName get_type_name() const;
    std::string get_display_group() const;
    bool has_display_name();
    std::string get_display_name() const;
    PXR_NS::VtTokenArray get_allowed_tokens() const;
    PXR_NS::VtTokenArray get_options() const;
    std::string get_documentation() const;
    PXR_NS::TfToken get_display_widget() const;
    PXR_NS::VtDictionary get_display_widget_hints() const;
    PXR_NS::UsdMetadataValueMap get_all_metadata() const;
    const std::vector<UsdPropertySource>& get_sources() const;
    void append_source(const UsdPropertySource& source);
    void append_source(const PXR_NS::TfToken& source_group, const PXR_NS::TfType& source_type);
    PXR_NS::UsdProperty get_property() const;
    PXR_NS::UsdAttribute get_attribute() const;
    bool is_relationship() const;
    PXR_NS::UsdRelationship get_relationship() const;
    PXR_NS::SdfSpecType get_type() const;
    PXR_NS::UsdPrim get_prim() const;
    bool is_authored() const;

    template <class T>
    bool get_metadata(const PXR_NS::TfToken& key, T& value) const
    {
        PXR_NS::VtValue data;
        if (get_metadata(key, data) && data.CanCast<T>())
        {
            value = data.Cast<T>().template Get<T>();
            return true;
        }
        return false;
    }
    bool get_metadata(const PXR_NS::TfToken& key, PXR_NS::VtValue& value) const;

    template <class T>
    bool set_metadata(const PXR_NS::TfToken& key, const T& value)
    {
        return set_metadata(key, PXR_NS::VtValue(value));
    }
    bool set_metadata(const PXR_NS::TfToken& key, const PXR_NS::VtValue& value);

private:
    friend class PropertyGatherer;

    PXR_NS::SdfSpecType m_type = PXR_NS::SdfSpecType::SdfSpecTypeUnknown;
    PXR_NS::UsdPrim m_prim;
    PXR_NS::UsdMetadataValueMap m_metadata;
    PXR_NS::TfToken m_name;
    PXR_NS::SdfPropertySpecHandle m_property_spec;
    std::vector<UsdPropertySource> m_sources;
};

using UsdPropertyProxyPtr = std::shared_ptr<UsdPropertyProxy>;
using UsdPropertyProxyVector = std::vector<UsdPropertyProxyPtr>;

OPENDCC_NAMESPACE_CLOSE
