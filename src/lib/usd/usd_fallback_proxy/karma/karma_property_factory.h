/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <usd_fallback_proxy/core/property_factory.h>

OPENDCC_NAMESPACE_OPEN

class KarmaPropertyFactory : public PropertyFactory
{
public:
    virtual ~KarmaPropertyFactory() override = default;
    virtual void get_properties(const PXR_NS::UsdPrim& prim, PropertyGatherer& property_gatherer) override;
    virtual void get_property(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& property_name, PropertyGatherer& property_gatherer) override;
    virtual bool is_prim_proxy_outdated(const PXR_NS::UsdPrim& prim, const PXR_NS::TfTokenVector& resynced_property_names,
                                        const PXR_NS::TfTokenVector& changed_property_names) const override;
    virtual PXR_NS::TfType get_type() const override;
};

OPENDCC_NAMESPACE_CLOSE
