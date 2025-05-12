// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/translator/registry.h"

OPENDCC_NAMESPACE_OPEN

void HydraOpTranslatorRegistry::register_node(const PXR_NS::TfToken& type, TranslatorFactoryFn translator_fn)
{
    m_registry[type] = translator_fn;
}

void HydraOpTranslatorRegistry::unregister_node(const PXR_NS::TfToken& type)
{
    m_registry.erase(type);
}

std::unique_ptr<HydraOpNodeTranslator> HydraOpTranslatorRegistry::make_translator(const PXR_NS::UsdPrim& prim) const
{
    if (auto entry = get_entry(prim); entry != m_registry.end())
    {
        return entry->second(prim);
    }

    return nullptr;
}

bool HydraOpTranslatorRegistry::has_translator(const PXR_NS::UsdPrim& prim) const
{
    return get_entry(prim) != m_registry.end();
}

HydraOpTranslatorRegistry& HydraOpTranslatorRegistry::instance()
{
    static HydraOpTranslatorRegistry registry;
    return registry;
}

HydraOpTranslatorRegistry::Registry::const_iterator HydraOpTranslatorRegistry::get_entry(const PXR_NS::UsdPrim& prim) const
{
    auto it = m_registry.find(prim.GetTypeName());
    if (it != m_registry.end())
    {
        return it;
    }

    for (const auto& schema : prim.GetAppliedSchemas())
    {
        it = m_registry.find(schema);
        if (it != m_registry.end())
        {
            return it;
        }
    }

    return m_registry.end();
}

OPENDCC_NAMESPACE_CLOSE
