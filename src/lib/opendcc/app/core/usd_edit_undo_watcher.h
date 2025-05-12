/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <functional>
#include <pxr/base/tf/weakBase.h>
#include <pxr/usd/usd/notice.h>
#include <pxr/usd/usd/stage.h>
#include <stack>
#include "opendcc/app/core/undo/inverse.h"
#include "opendcc/app/core/undo/router.h"
#include "opendcc/usd/layer_tree_watcher/layer_state_delegates_holder.h"

OPENDCC_NAMESPACE_OPEN
using UsdEditUndoStackChanged = std::function<void(std::shared_ptr<commands::UndoInverse>)>;

class UsdEditUndoStackChangedWatcher : public PXR_NS::TfWeakBase
{
public:
    UsdEditUndoStackChangedWatcher(std::shared_ptr<LayerTreeWatcher> layer_tree, std::shared_ptr<LayerStateDelegatesHolder> layer_state_delegates,
                                   const UsdEditUndoStackChanged& callback);
    virtual ~UsdEditUndoStackChangedWatcher();

private:
    void on_stack_changed(const commands::UndoStackNotice& notice);

    PXR_NS::TfNotice::Key m_undo_stack_changed_key;
    LayerTreeWatcher::SublayersChangedDispatcherHandle m_sublayers_changed_key;
    UsdEditUndoStackChanged m_callback;
    std::stack<std::shared_ptr<commands::UndoInverse>> m_undo_stack;
    std::shared_ptr<LayerTreeWatcher> m_layer_tree;
    std::shared_ptr<LayerStateDelegatesHolder> m_layer_state_delegates;
};
OPENDCC_NAMESPACE_CLOSE
