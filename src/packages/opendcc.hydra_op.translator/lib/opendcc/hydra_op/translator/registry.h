/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/hydra_op/translator/api.h"
#include <pxr/usd/usd/schemaBase.h>
#include <pxr/imaging/hd/sceneIndex.h>
#include "opendcc/hydra_op/translator/network.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_HYDRA_OP_TRANSLATOR_API HydraOpTranslatorRegistry final
{
public:
    using TranslatorFactoryFn = std::function<std::unique_ptr<HydraOpNodeTranslator>(const PXR_NS::UsdPrim&)>;
    void register_node(const PXR_NS::TfToken& type, TranslatorFactoryFn translator_fn);
    void unregister_node(const PXR_NS::TfToken& type);

    std::unique_ptr<HydraOpNodeTranslator> make_translator(const PXR_NS::UsdPrim& prim) const;
    bool has_translator(const PXR_NS::UsdPrim& prim) const;
    static HydraOpTranslatorRegistry& instance();

private:
    HydraOpTranslatorRegistry() = default;
    HydraOpTranslatorRegistry(const HydraOpTranslatorRegistry&) = delete;
    HydraOpTranslatorRegistry(HydraOpTranslatorRegistry&&) = delete;
    HydraOpTranslatorRegistry& operator=(const HydraOpTranslatorRegistry&) = delete;
    HydraOpTranslatorRegistry& operator=(HydraOpTranslatorRegistry&&) = delete;

    using Registry = std::unordered_map<PXR_NS::TfToken, TranslatorFactoryFn, PXR_NS::TfToken::HashFunctor>;
    Registry::const_iterator get_entry(const PXR_NS::UsdPrim& prim) const;

    std::unordered_map<PXR_NS::TfToken, TranslatorFactoryFn, PXR_NS::TfToken::HashFunctor> m_registry;
};

OPENDCC_NAMESPACE_CLOSE
