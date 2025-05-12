// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/usd_live_share/live_share_state_delegate.h"
#include "opendcc/usd/usd_ipc_serialization/usd_edits.h"
#include "opendcc/usd/usd_ipc_serialization/usd_ipc_utils.h"
#include "opendcc/usd/layer_tree_watcher/layer_state_delegates_holder.h"
#include "opendcc/usd/usd_live_share/live_share_edits.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_PRIVATE_TOKENS(ShareEditsDelegateTokens, (LiveShareDelegate));

LiveShareStateDelegate::LiveShareStateDelegate(LayerStateDelegateProxyPtr state_delegate_proxy, ShareEditsContext* share_context)
    : m_state_delegate_proxy(state_delegate_proxy)
    , m_share_context(share_context)
{
}

TfToken LiveShareStateDelegate::get_name()
{
    return ShareEditsDelegateTokens->LiveShareDelegate;
}

bool LiveShareStateDelegate::_IsDirty()
{
    return m_state_delegate_proxy->IsDirty();
}

void LiveShareStateDelegate::_MarkCurrentStateAsClean()
{
    // empty
}

void LiveShareStateDelegate::_MarkCurrentStateAsDirty()
{
    // empty
}

void LiveShareStateDelegate::_OnSetLayer(const SdfLayerHandle& layer) {}

void LiveShareStateDelegate::_OnSetField(const SdfPath& path, const TfToken& field_name, const VtValue& value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(std::make_unique<UsdEditSetField>(m_state_delegate_proxy->get_layer()->GetIdentifier(), path, field_name, value));
    }
}

void LiveShareStateDelegate::_OnSetField(const SdfPath& path, const TfToken& field_name, const SdfAbstractDataConstValue& value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(std::make_unique<UsdEditSetField>(m_state_delegate_proxy->get_layer()->GetIdentifier(), path, field_name, value));
    }
}

void LiveShareStateDelegate::_OnSetFieldDictValueByKey(const SdfPath& path, const TfToken& field_name, const TfToken& key_path, const VtValue& value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(
            std::make_unique<UsdEditSetFieldDictValueByKey>(m_state_delegate_proxy->get_layer()->GetIdentifier(), path, field_name, key_path, value));
    }
}

void LiveShareStateDelegate::_OnSetFieldDictValueByKey(const SdfPath& path, const TfToken& field_name, const TfToken& key_path,
                                                       const SdfAbstractDataConstValue& value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(
            std::make_unique<UsdEditSetFieldDictValueByKey>(m_state_delegate_proxy->get_layer()->GetIdentifier(), path, field_name, key_path, value));
    }
}

void LiveShareStateDelegate::_OnSetTimeSample(const SdfPath& path, double time, const VtValue& value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(std::make_unique<UsdEditSetTimesample>(m_state_delegate_proxy->get_layer()->GetIdentifier(), path, time, value));
    }
}

void LiveShareStateDelegate::_OnSetTimeSample(const SdfPath& path, double time, const SdfAbstractDataConstValue& value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(std::make_unique<UsdEditSetTimesample>(m_state_delegate_proxy->get_layer()->GetIdentifier(), path, time, value));
    }
}

void LiveShareStateDelegate::_OnCreateSpec(const SdfPath& path, SdfSpecType spec_type, bool inert)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(std::make_unique<UsdEditCreateSpec>(m_state_delegate_proxy->get_layer()->GetIdentifier(), path, spec_type, inert));
    }
}

void LiveShareStateDelegate::_OnDeleteSpec(const SdfPath& path, bool inert)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(std::make_unique<UsdEditDeleteSpec>(m_state_delegate_proxy->get_layer()->GetIdentifier(), path, inert));
    }
}

void LiveShareStateDelegate::_OnMoveSpec(const SdfPath& old_path, const SdfPath& new_path)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(std::make_unique<UsdEditMoveSpec>(m_state_delegate_proxy->get_layer()->GetIdentifier(), old_path, new_path));
    }
}

void LiveShareStateDelegate::_OnPushChild(const SdfPath& parent_path, const TfToken& field_name, const TfToken& value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(
            std::make_unique<UsdEditPushChild>(m_state_delegate_proxy->get_layer()->GetIdentifier(), parent_path, field_name, value));
    }
}

void LiveShareStateDelegate::_OnPushChild(const SdfPath& parent_path, const TfToken& field_name, const SdfPath& value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(
            std::make_unique<UsdEditPushChild>(m_state_delegate_proxy->get_layer()->GetIdentifier(), parent_path, field_name, value));
    }
}

void LiveShareStateDelegate::_OnPopChild(const SdfPath& parent_path, const TfToken& field_name, const TfToken& old_value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(
            std::make_unique<UsdEditPopChild>(m_state_delegate_proxy->get_layer()->GetIdentifier(), parent_path, field_name, old_value));
    }
}

void LiveShareStateDelegate::_OnPopChild(const SdfPath& parent_path, const TfToken& field_name, const SdfPath& old_value)
{
    m_state_delegate_proxy->set_dirty(true);
    if (!m_share_context->is_processing_incoming_edits())
    {
        m_share_context->send_edit(
            std::make_unique<UsdEditPopChild>(m_state_delegate_proxy->get_layer()->GetIdentifier(), parent_path, field_name, old_value));
    }
}
OPENDCC_NAMESPACE_CLOSE
