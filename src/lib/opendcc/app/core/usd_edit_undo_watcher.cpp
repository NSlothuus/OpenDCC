// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/usd_edit_undo_watcher.h"
#include "opendcc/app/core/undo/router.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include <pxr/usd/usd/stage.h>
#include "opendcc/app/core/undo/state_delegate.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE
using namespace commands;

UsdEditUndoStackChangedWatcher::UsdEditUndoStackChangedWatcher(std::shared_ptr<LayerTreeWatcher> layer_tree,
                                                               std::shared_ptr<LayerStateDelegatesHolder> layer_state_delegates,
                                                               const UsdEditUndoStackChanged& callback)
    : m_layer_tree(layer_tree)
    , m_layer_state_delegates(layer_state_delegates)
    , m_callback(callback)
{
    m_undo_stack_changed_key = TfNotice::Register(TfWeakPtr<UsdEditUndoStackChangedWatcher>(this), &UsdEditUndoStackChangedWatcher::on_stack_changed);
    m_sublayers_changed_key = m_layer_tree->register_sublayers_changed_callback(
        [this](std::string layer, std::string parent, LayerTreeWatcher::SublayerChangeType change_type) {
            if (change_type == LayerTreeWatcher::SublayerChangeType::Added)
                m_layer_state_delegates->add_delegate(commands::UndoStateDelegate::get_name(), layer);
        });
    m_layer_state_delegates->add_delegate(commands::UndoStateDelegate::get_name());
}

UsdEditUndoStackChangedWatcher::~UsdEditUndoStackChangedWatcher()
{
    m_layer_tree->unregister_sublayers_changed_callback(m_sublayers_changed_key);
}

void UsdEditUndoStackChangedWatcher::on_stack_changed(const UndoStackNotice& notice)
{
    m_callback(notice.get_inverse());
}
OPENDCC_NAMESPACE_CLOSE
