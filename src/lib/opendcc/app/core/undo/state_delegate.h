/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/usd/sdf/data.h>
#include "opendcc/usd/layer_tree_watcher/layer_state_delegates_registry.h"
OPENDCC_NAMESPACE_OPEN
namespace commands
{
    class UndoStateDelegate : public LayerStateDelegate
    {
    public:
        UndoStateDelegate(LayerStateDelegateProxyPtr state_delegate_proxy);
        static PXR_NS::TfToken get_name();

    private:
        LayerStateDelegateProxyPtr m_state_delegate_proxy;

    protected:
        virtual bool _IsDirty() override;
        virtual void _MarkCurrentStateAsClean() override;
        virtual void _MarkCurrentStateAsDirty() override;
        virtual void _OnSetLayer(const PXR_NS::SdfLayerHandle& layer) override;
        virtual void _OnSetField(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name, const PXR_NS::VtValue& value) override;
        virtual void _OnSetField(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name,
                                 const PXR_NS::SdfAbstractDataConstValue& value) override;
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

        static bool invert_set_field(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name,
                                     const PXR_NS::VtValue& inverse);
        static bool invert_set_field_dict_value_by_key(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& path,
                                                       const PXR_NS::TfToken& field_name, const PXR_NS::TfToken& key_path,
                                                       const PXR_NS::VtValue& inverse);
        static bool invert_set_time_sample(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& path, double time, const PXR_NS::VtValue& value);
        static bool invert_create_spec(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& path, bool inert);
        static bool invert_delete_spec(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& path, bool inert,
                                       PXR_NS::SdfSpecType deleted_spec_type, const PXR_NS::SdfDataRefPtr& deleted_data);
        static bool invert_move_spec(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& oldPath, const PXR_NS::SdfPath& newPath);
        static bool invert_push_token_child(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& parentPath, const PXR_NS::TfToken& fieldName,
                                            const PXR_NS::TfToken& value);
        static bool invert_pop_token_child(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& parentPath, const PXR_NS::TfToken& fieldName,
                                           const PXR_NS::TfToken& oldValue);
        static bool invert_push_path_child(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& parentPath, const PXR_NS::TfToken& fieldName,
                                           const PXR_NS::SdfPath& value);
        static bool invert_pop_path_child(LayerStateDelegateProxyPtr proxy, const PXR_NS::SdfPath& parentPath, const PXR_NS::TfToken& fieldName,
                                          const PXR_NS::SdfPath& oldValue);

    private:
        void on_set_field_impl(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name);
        void on_set_field_dict_value_by_key_impl(const PXR_NS::SdfPath& path, const PXR_NS::TfToken& field_name, const PXR_NS::TfToken& key_path);
        void on_set_time_sample_impl(const PXR_NS::SdfPath& path, double time);
    };

} // namespace commands
OPENDCC_NAMESPACE_CLOSE
