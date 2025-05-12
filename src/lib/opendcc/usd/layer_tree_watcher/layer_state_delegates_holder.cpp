// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/layer_tree_watcher/layer_state_delegates_holder.h"
#include "opendcc/usd/layer_tree_watcher/layer_state_delegates_registry.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_PRIVATE_TOKENS(LayerStateDelegateTokens, (LayerStateDelegateProxy));

LayerStateDelegateProxyRefPtr LayerStateDelegateProxy::create()
{
    return TfCreateRefPtr(new LayerStateDelegateProxy);
}

void LayerStateDelegateProxy::add_delegate(const TfToken& delegate_name, std::shared_ptr<LayerStateDelegate> delegate)
{
    m_delegates[delegate_name] = delegate;
    delegate->_OnSetLayer(this->_GetLayer());
}
void LayerStateDelegateProxy::remove_delegate(const TfToken& delegate_name)
{
    m_delegates.erase(delegate_name);
}

bool LayerStateDelegateProxy::_IsDirty()
{
    return m_is_dirty;
}
void LayerStateDelegateProxy::_MarkCurrentStateAsClean()
{
    m_is_dirty = false;
    for_each(&LayerStateDelegate::_MarkCurrentStateAsClean);
}
void LayerStateDelegateProxy::_MarkCurrentStateAsDirty()
{
    m_is_dirty = true;
    for_each(&LayerStateDelegate::_MarkCurrentStateAsDirty);
}
void LayerStateDelegateProxy::_OnSetLayer(const SdfLayerHandle& layer)
{
    for_each(&LayerStateDelegate::_OnSetLayer, layer);
}
void LayerStateDelegateProxy::_OnSetField(const SdfPath& path, const TfToken& field_name, const VtValue& value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, const TfToken&, const VtValue&)) & LayerStateDelegate::_OnSetField, path, field_name,
             value);
}
void LayerStateDelegateProxy::_OnSetField(const SdfPath& path, const TfToken& field_name, const SdfAbstractDataConstValue& value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, const TfToken&, const SdfAbstractDataConstValue&)) & LayerStateDelegate::_OnSetField, path,
             field_name, value);
}
void LayerStateDelegateProxy::_OnSetFieldDictValueByKey(const SdfPath& path, const TfToken& field_name, const TfToken& key_path, const VtValue& value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, const TfToken&, const TfToken&, const VtValue&)) &
                 LayerStateDelegate::_OnSetFieldDictValueByKey,
             path, field_name, key_path, value);
}
void LayerStateDelegateProxy::_OnSetFieldDictValueByKey(const SdfPath& path, const TfToken& field_name, const TfToken& key_path,
                                                        const SdfAbstractDataConstValue& value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, const TfToken&, const TfToken&, const SdfAbstractDataConstValue&)) &
                 LayerStateDelegate::_OnSetFieldDictValueByKey,
             path, field_name, key_path, value);
}
void LayerStateDelegateProxy::_OnSetTimeSample(const SdfPath& path, double time, const VtValue& value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, double, const VtValue&)) & LayerStateDelegate::_OnSetTimeSample, path, time, value);
}
void LayerStateDelegateProxy::_OnSetTimeSample(const SdfPath& path, double time, const SdfAbstractDataConstValue& value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, double, const SdfAbstractDataConstValue&)) & LayerStateDelegate::_OnSetTimeSample, path,
             time, value);
}
void LayerStateDelegateProxy::_OnCreateSpec(const SdfPath& path, SdfSpecType spec_type, bool inert)
{
    for_each(&LayerStateDelegate::_OnCreateSpec, path, spec_type, inert);
}
void LayerStateDelegateProxy::_OnDeleteSpec(const SdfPath& path, bool inert)
{
    for_each(&LayerStateDelegate::_OnDeleteSpec, path, inert);
}
void LayerStateDelegateProxy::_OnMoveSpec(const SdfPath& old_path, const SdfPath& new_path)
{
    for_each(&LayerStateDelegate::_OnMoveSpec, old_path, new_path);
}
void LayerStateDelegateProxy::_OnPushChild(const SdfPath& parent_path, const TfToken& field_name, const TfToken& value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, const TfToken&, const TfToken&)) & LayerStateDelegate::_OnPushChild, parent_path,
             field_name, value);
}
void LayerStateDelegateProxy::_OnPushChild(const SdfPath& parent_path, const TfToken& field_name, const SdfPath& value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, const TfToken&, const SdfPath&)) & LayerStateDelegate::_OnPushChild, parent_path,
             field_name, value);
}
void LayerStateDelegateProxy::_OnPopChild(const SdfPath& parent_path, const TfToken& field_name, const TfToken& old_value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, const TfToken&, const TfToken&)) & LayerStateDelegate::_OnPopChild, parent_path, field_name,
             old_value);
}
void LayerStateDelegateProxy::_OnPopChild(const SdfPath& parent_path, const TfToken& field_name, const SdfPath& old_value)
{
    for_each((void(LayerStateDelegate::*)(const SdfPath&, const TfToken&, const SdfPath&)) & LayerStateDelegate::_OnPopChild, parent_path, field_name,
             old_value);
}

template <class TFn, class... TArgs>
void LayerStateDelegateProxy::for_each(TFn fn_member, TArgs&&... args)
{
    for (auto& delegate : m_delegates)
    {
        ((*delegate.second).*fn_member)(std::forward<TArgs>(args)...);
    }
}

PXR_NS::SdfAbstractDataPtr LayerStateDelegateProxy::get_layer_data() const
{
    return _GetLayerData();
}

PXR_NS::SdfLayerHandle LayerStateDelegateProxy::get_layer() const
{
    return _GetLayer();
}

void LayerStateDelegateProxy::set_dirty(bool dirty)
{
    if (dirty == m_is_dirty)
        return;

    dirty ? _MarkCurrentStateAsDirty() : _MarkCurrentStateAsClean();
}

LayerStateDelegateProxy::~LayerStateDelegateProxy()
{
    auto stop = 5;
}

LayerStateDelegatesHolder::LayerStateDelegatesHolder(std::shared_ptr<LayerTreeWatcher> layer_tree)
    : m_layer_tree(layer_tree)
{
    for (const auto& layer : m_layer_tree->get_all_layers())
    {
        auto state_delegate_holder = TfDynamic_cast<LayerStateDelegateProxyRefPtr>(layer->GetStateDelegate());
        if (!state_delegate_holder)
            layer->SetStateDelegate(LayerStateDelegateProxy::create());
    }
}

void LayerStateDelegatesHolder::add_delegate(const TfToken& delegate_name)
{
    for (const auto& layer : m_layer_tree->get_all_layers())
    {
        auto proxy_delegate = get_delegate_proxy(layer);
        auto new_delegate = LayerStateDelegateRegistry::create(delegate_name, TfWeakPtr<LayerStateDelegateProxy>(proxy_delegate));
        proxy_delegate->add_delegate(delegate_name, new_delegate);
    }
}

void LayerStateDelegatesHolder::add_delegate(const TfToken& delegate_name, const std::string& identifier)
{
    auto layer = m_layer_tree->get_layer(identifier);
    if (!layer)
        return;

    auto proxy_delegate = get_delegate_proxy(layer);
    auto new_delegate = LayerStateDelegateRegistry::create(delegate_name, TfWeakPtr<LayerStateDelegateProxy>(proxy_delegate));
    proxy_delegate->add_delegate(delegate_name, new_delegate);
}

void LayerStateDelegatesHolder::remove_delegate(const TfToken& delegate_name)
{
    for (const auto& layer : m_layer_tree->get_all_layers())
        get_delegate_proxy(layer)->remove_delegate(delegate_name);
}

void LayerStateDelegatesHolder::remove_delegate(const TfToken& delegate_name, const std::string& identifier)
{
    auto layer = m_layer_tree->get_layer(identifier);
    if (!layer)
        return;
    get_delegate_proxy(layer)->remove_delegate(delegate_name);
}

LayerStateDelegateProxyRefPtr LayerStateDelegatesHolder::get_delegate_proxy(SdfLayerHandle layer)
{
    auto state_delegate_holder = TfDynamic_cast<LayerStateDelegateProxyRefPtr>(layer->GetStateDelegate());
    if (state_delegate_holder)
        return state_delegate_holder;

    state_delegate_holder = LayerStateDelegateProxy::create();
    layer->SetStateDelegate(state_delegate_holder);
    return state_delegate_holder;
}
OPENDCC_NAMESPACE_CLOSE
