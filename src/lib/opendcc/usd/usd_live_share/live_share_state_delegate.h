/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/usd/layer_tree_watcher/layer_state_delegates_registry.h"

OPENDCC_NAMESPACE_OPEN
class ShareEditsContext;

class LiveShareStateDelegate : public LayerStateDelegate
{
public:
    LiveShareStateDelegate(LayerStateDelegateProxyPtr state_delegate_proxy, ShareEditsContext* share_context);
    static PXR_NS::TfToken get_name();

private:
    LayerStateDelegateProxyPtr m_state_delegate_proxy;
    ShareEditsContext* m_share_context = nullptr;

protected:
    virtual bool _IsDirty() override;
    virtual void _MarkCurrentStateAsClean() override;
    virtual void _MarkCurrentStateAsDirty() override;
    virtual void _OnSetLayer(const PXR_NS::SdfLayerHandle& layer) override;
    virtual void _OnSetField(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name, const PXR_NS::VtValue& value) override;
    virtual void _OnSetField(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name, const PXR_NS::SdfAbstractDataConstValue& value) override;
    virtual void _OnSetFieldDictValueByKey(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& fieldName, const PXR_NS::TfToken& keyPath,
                                           const PXR_NS::VtValue& value) override;
    virtual void _OnSetFieldDictValueByKey(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& fieldName, const PXR_NS::TfToken& keyPath,
                                           const PXR_NS::SdfAbstractDataConstValue& value) override;
    virtual void _OnSetTimeSample(const PXR_NS::SdfPath& path, double time, const PXR_NS::VtValue& value) override;
    virtual void _OnSetTimeSample(const PXR_NS::SdfPath& path, double time, const PXR_NS::SdfAbstractDataConstValue& value) override;
    virtual void _OnCreateSpec(const PXR_NS::SdfPath& path, PXR_NS::SdfSpecType specType, bool inert) override;
    virtual void _OnDeleteSpec(const PXR_NS::SdfPath& path, bool inert) override;
    virtual void _OnMoveSpec(const PXR_NS::SdfPath& oldPath, const PXR_NS::SdfPath& newPath) override;
    virtual void _OnPushChild(const PXR_NS::SdfPath& parentPath, const PXR_NS::TfToken& fieldName, const PXR_NS::TfToken& value) override;
    virtual void _OnPushChild(const PXR_NS::SdfPath& parentPath, const PXR_NS::TfToken& fieldName, const PXR_NS::SdfPath& value) override;
    virtual void _OnPopChild(const PXR_NS::SdfPath& parentPath, const PXR_NS::TfToken& fieldName, const PXR_NS::TfToken& oldValue) override;
    virtual void _OnPopChild(const PXR_NS::SdfPath& parentPath, const PXR_NS::TfToken& fieldName, const PXR_NS::SdfPath& oldValue) override;
};
OPENDCC_NAMESPACE_CLOSE
