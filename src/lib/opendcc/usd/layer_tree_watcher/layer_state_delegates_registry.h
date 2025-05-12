/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/usd/layer_tree_watcher/api.h"
#include "opendcc/usd/layer_tree_watcher/layer_state_delegates_holder.h"
#include <pxr/usd/sdf/layerStateDelegate.h>
#include <unordered_map>
OPENDCC_NAMESPACE_OPEN
class LayerStateDelegateProxy;
class USD_LAYER_TREE_WATCHER_API LayerStateDelegate : public PXR_NS::SdfLayerStateDelegateBase
{
    friend class LayerStateDelegateProxy;
};
using LayerStateDelegatePtr = std::shared_ptr<LayerStateDelegate>;

class USD_LAYER_TREE_WATCHER_API LayerStateDelegateRegistry
{
public:
    static bool register_state_delegate(const PXR_NS::TfToken& name,
                                        const std::function<LayerStateDelegatePtr(LayerStateDelegateProxyPtr)>& create_fn);
    static bool unregister_state_delegate(const PXR_NS::TfToken& name);

    static LayerStateDelegatePtr create(const PXR_NS::TfToken& name, LayerStateDelegateProxyPtr proxy);

    static LayerStateDelegateRegistry& instance();

private:
    LayerStateDelegateRegistry() = default;

    std::unordered_map<PXR_NS::TfToken, std::function<LayerStateDelegatePtr(LayerStateDelegateProxyPtr)>, PXR_NS::TfToken::HashFunctor> m_registry;
};

#define OPENDCC_REGISTER_LAYER_STATE_DELEGATE(name, state_delegate_type)                                                    \
    static bool state_delegate_type##layer_state_delegate_registered = LayerStateDelegateRegistry::register_state_delegate( \
        name, [](LayerStateDelegateProxyPtr proxy) { return std::make_shared<state_delegate_type>(proxy); });
OPENDCC_NAMESPACE_CLOSE
