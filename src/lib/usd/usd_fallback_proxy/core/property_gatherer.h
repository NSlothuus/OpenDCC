/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <usd/usd_fallback_proxy/core/property_factory.h>
#include <usd/usd_fallback_proxy/core/usd_property_proxy.h>
#include <usd/usd_fallback_proxy/core/usd_property_source.h>

OPENDCC_NAMESPACE_OPEN

class SourceRegistry;

class FALLBACK_PROXY_API PropertyGatherer
{
public:
    PropertyGatherer() = default;

    bool contains(const PXR_NS::TfToken& property_name) const;

    bool try_insert_property(PXR_NS::SdfSpecType spec_type, const PXR_NS::TfToken& property_name, const PXR_NS::UsdPrim& prim,
                             const PXR_NS::UsdMetadataValueMap& metadata, const UsdPropertySource& source = UsdPropertySource(),
                             PXR_NS::SdfPropertySpecHandle property_spec = PXR_NS::SdfPropertySpecHandle());

    const PXR_NS::UsdMetadataValueMap& get_metadata(const PXR_NS::TfToken& property_name) const;
    bool update_metadata(const PXR_NS::TfToken& property_name, const PXR_NS::UsdMetadataValueMap& metadata);

private:
    friend class SourceRegistry;

    UsdPropertyProxyVector m_all_properties;
    UsdPropertyProxyVector m_current_properties;
};
OPENDCC_NAMESPACE_CLOSE
