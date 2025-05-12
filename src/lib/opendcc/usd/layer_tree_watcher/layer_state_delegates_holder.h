/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/usd/sdf/layerStateDelegate.h>
#include "opendcc/usd/layer_tree_watcher/layer_tree_watcher.h"

OPENDCC_NAMESPACE_OPEN

class LayerStateDelegateProxy;
using LayerStateDelegateProxyRefPtr = PXR_NS::TfDeclarePtrs<class LayerStateDelegateProxy>::RefPtr;
using LayerStateDelegateProxyPtr = PXR_NS::TfDeclarePtrs<class LayerStateDelegateProxy>::Ptr;
class LayerStateDelegate;

class USD_LAYER_TREE_WATCHER_API LayerStateDelegateProxy final : public PXR_NS::SdfLayerStateDelegateBase
{
public:
    static LayerStateDelegateProxyRefPtr create();

    void add_delegate(const PXR_NS::TfToken& delegate_name, std::shared_ptr<LayerStateDelegate> delegate);
    void remove_delegate(const PXR_NS::TfToken& delegate_name);
    PXR_NS::SdfLayerHandle get_layer() const;
    PXR_NS::SdfAbstractDataPtr get_layer_data() const;
    void set_dirty(bool dirty);
    ~LayerStateDelegateProxy();

protected:
    LayerStateDelegateProxy() = default;

    virtual bool _IsDirty() override;
    virtual void _MarkCurrentStateAsClean() override;
    virtual void _MarkCurrentStateAsDirty() override;
    virtual void _OnSetLayer(const PXR_NS::SdfLayerHandle& layer) override;
    virtual void _OnSetField(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name, const PXR_NS::VtValue& value) override;
    virtual void _OnSetField(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name, const PXR_NS::SdfAbstractDataConstValue& value) override;
    virtual void _OnSetFieldDictValueByKey(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name, const PXR_NS::TfToken& key_path,
                                           const PXR_NS::VtValue& value) override;
    virtual void _OnSetFieldDictValueByKey(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name, const PXR_NS::TfToken& key_path,
                                           const PXR_NS::SdfAbstractDataConstValue& value) override;
    virtual void _OnSetTimeSample(const PXR_NS::SdfPath& path, double time, const PXR_NS::VtValue& value) override;
    virtual void _OnSetTimeSample(const PXR_NS::SdfPath& path, double time, const PXR_NS::SdfAbstractDataConstValue& value) override;
    virtual void _OnCreateSpec(const PXR_NS::SdfPath& path, PXR_NS::SdfSpecType spec_type, bool inert) override;
    virtual void _OnDeleteSpec(const PXR_NS::SdfPath& path, bool inert) override;
    virtual void _OnMoveSpec(const PXR_NS::SdfPath& old_path, const PXR_NS::SdfPath& new_path) override;
    virtual void _OnPushChild(const PXR_NS::SdfPath& parent_path, const PXR_NS::TfToken& field_name, const PXR_NS::TfToken& value) override;
    virtual void _OnPushChild(const PXR_NS::SdfPath& parent_path, const PXR_NS::TfToken& field_name, const PXR_NS::SdfPath& value) override;
    virtual void _OnPopChild(const PXR_NS::SdfPath& parent_path, const PXR_NS::TfToken& field_name, const PXR_NS::TfToken& old_value) override;
    virtual void _OnPopChild(const PXR_NS::SdfPath& parent_path, const PXR_NS::TfToken& field_name, const PXR_NS::SdfPath& old_value) override;

private:
    template <class TFn, class... TArgs>
    void for_each(TFn fn_member, TArgs&&... args);

    std::unordered_map<PXR_NS::TfToken, std::shared_ptr<LayerStateDelegate>, PXR_NS::TfToken::HashFunctor> m_delegates;
    bool m_is_dirty = false;
};

class USD_LAYER_TREE_WATCHER_API LayerStateDelegatesHolder
{
public:
    LayerStateDelegatesHolder(std::shared_ptr<LayerTreeWatcher> layer_tree);

    void add_delegate(const PXR_NS::TfToken& delegate_name);
    void remove_delegate(const PXR_NS::TfToken& delegate_name);
    void add_delegate(const PXR_NS::TfToken& delegate_name, const std::string& identifier);
    void remove_delegate(const PXR_NS::TfToken& delegate_name, const std::string& identifier);

private:
    LayerStateDelegateProxyRefPtr get_delegate_proxy(PXR_NS::SdfLayerHandle layer);

    std::shared_ptr<LayerTreeWatcher> m_layer_tree;
};

OPENDCC_NAMESPACE_CLOSE
