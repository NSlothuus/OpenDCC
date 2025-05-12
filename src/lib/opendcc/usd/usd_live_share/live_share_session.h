/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/usd/usd_live_share/api.h"
#include "opendcc/usd/usd_live_share/live_share_edits.h"
#include "opendcc/usd/layer_tree_watcher/layer_tree_watcher.h"
#include "opendcc/usd/layer_tree_watcher/layer_state_delegates_holder.h"

OPENDCC_NAMESPACE_OPEN
class USD_LIVE_SHARE_API LiveShareSession
{
public:
    using ConnectionSettings = ShareEditsContext::ConnectionSettings;
    LiveShareSession(PXR_NS::UsdStageRefPtr stage, const ConnectionSettings& connection_settings = ConnectionSettings());
    ~LiveShareSession();
    void start_share();
    void stop_share();
    void process();

private:
    PXR_NS::UsdStageRefPtr m_stage;
    std::string m_layer_transfer_path;
    std::unique_ptr<ShareEditsContext> m_context;
    std::shared_ptr<LayerTreeWatcher> m_layer_tree_watcher;
    std::shared_ptr<LayerStateDelegatesHolder> m_layer_state_delegates;
    ConnectionSettings m_connection_settings;
};
OPENDCC_NAMESPACE_CLOSE
