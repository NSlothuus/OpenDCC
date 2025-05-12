/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <usd_fallback_proxy/core/property_factory.h>

OPENDCC_NAMESPACE_OPEN

class UsdPrimPropertyFactory : public PropertyFactory
{
public:
    UsdPrimPropertyFactory() = default;
    virtual ~UsdPrimPropertyFactory() override = default;

    virtual void get_properties(const PXR_NS::UsdPrim& prim, PropertyGatherer& property_gatherer) override;
    virtual void get_property(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& attribute_name, PropertyGatherer& property_gatherer) override;
    virtual bool is_prim_proxy_outdated(const PXR_NS::UsdPrim& prim, const PXR_NS::TfTokenVector& resynced_property_names,
                                        const PXR_NS::TfTokenVector& changed_property_names) const override;
    virtual PXR_NS::TfType get_type() const override;

private:
#if PXR_VERSION >= 2005
    std::vector<const PXR_NS::UsdPrimDefinition*> get_schemas_definitions(const PXR_NS::UsdPrim& prim) const;
#else
    PXR_NS::TfTokenVector get_prim_schemas(const PXR_NS::UsdPrim& prim) const;
#endif
};

OPENDCC_NAMESPACE_CLOSE