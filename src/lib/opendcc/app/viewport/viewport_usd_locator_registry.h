/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_usd_locator.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API ViewportUsdLocatorRegistry
{
public:
    using LocatorFactoryFn = std::function<ViewportUsdLocatorPtr(ViewportLocatorDelegate* delegate, const PXR_NS::UsdPrim& prim)>;

    static ViewportUsdLocatorPtr create_locator(ViewportLocatorDelegate* delegate, const PXR_NS::UsdPrim& prim);
    static bool register_locator_factory(const PXR_NS::TfToken& type, LocatorFactoryFn fn);
    static bool unregister_locator_factory(const PXR_NS::TfToken& type);
    static bool has_factory(const PXR_NS::TfToken& type);

private:
    ViewportUsdLocatorRegistry() = default;
    ViewportUsdLocatorRegistry(const ViewportUsdLocatorRegistry&) = delete;
    ViewportUsdLocatorRegistry(ViewportUsdLocatorRegistry&&) = delete;
    ViewportUsdLocatorRegistry& operator=(const ViewportUsdLocatorRegistry&) = delete;
    ViewportUsdLocatorRegistry& operator=(ViewportUsdLocatorRegistry&&) = delete;

    static ViewportUsdLocatorRegistry& instance();

    std::unordered_map<PXR_NS::TfToken, LocatorFactoryFn, PXR_NS::TfToken::HashFunctor> m_registry;
};

#define OPENDCC_REGISTER_USD_LOCATOR(LocatorClass, type)                                                        \
    TF_REGISTRY_FUNCTION(TfType)                                                                                \
    {                                                                                                           \
        ViewportUsdLocatorRegistry::register_locator_factory(                                                   \
            type, [](ViewportLocatorDelegate* delegate, const PXR_NS::UsdPrim& prim) -> ViewportUsdLocatorPtr { \
                return std::make_shared<LocatorClass>(delegate, prim);                                          \
            });                                                                                                 \
    }

OPENDCC_NAMESPACE_CLOSE