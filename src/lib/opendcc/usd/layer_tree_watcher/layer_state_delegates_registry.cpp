// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "layer_state_delegates_registry.h"
OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

bool LayerStateDelegateRegistry::register_state_delegate(const TfToken& name,
                                                         const std::function<LayerStateDelegatePtr(LayerStateDelegateProxyPtr)>& create_fn)
{
    auto& self = instance();
    auto iter = self.m_registry.find(name);
    if (iter != self.m_registry.end())
        return false;
    self.m_registry[name] = create_fn;
    return true;
}

bool LayerStateDelegateRegistry::unregister_state_delegate(const TfToken& name)
{
    return instance().m_registry.erase(name) == 1;
}

LayerStateDelegatePtr LayerStateDelegateRegistry::create(const TfToken& name, LayerStateDelegateProxyPtr proxy)
{
    auto& self = instance();
    auto iter = self.m_registry.find(name);
    return iter == self.m_registry.end() ? nullptr : iter->second(proxy);
}

LayerStateDelegateRegistry& LayerStateDelegateRegistry::instance()
{
    static LayerStateDelegateRegistry instance;
    return instance;
}
OPENDCC_NAMESPACE_CLOSE
