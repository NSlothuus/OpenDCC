/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "usd/usd_fallback_proxy/core/api.h"
#include <pxr/usd/usd/prim.h>

OPENDCC_NAMESPACE_OPEN

class PropertyGatherer;

class FALLBACK_PROXY_API PropertyFactory
{
public:
    PropertyFactory() = default;
    virtual ~PropertyFactory() = default;

    virtual void get_properties(const PXR_NS::UsdPrim& prim, PropertyGatherer& property_gatherer) = 0;
    virtual void get_property(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& attribute_name, PropertyGatherer& property_gatherer) = 0;
    virtual bool is_prim_proxy_outdated(const PXR_NS::UsdPrim& prim, const PXR_NS::TfTokenVector& resynced_property_names,
                                        const PXR_NS::TfTokenVector& changed_property_names) const = 0;
    virtual PXR_NS::TfType get_type() const = 0;
};

using PropertyFactoryPtr = std::unique_ptr<PropertyFactory>;

OPENDCC_NAMESPACE_CLOSE
