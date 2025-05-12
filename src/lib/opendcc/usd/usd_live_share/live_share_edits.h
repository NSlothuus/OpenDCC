/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/usd/usd_live_share/api.h"
#include "opendcc/usd/layer_tree_watcher/layer_tree_watcher.h"
#include "opendcc/usd/usd_ipc_serialization/usd_edits.h"
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include <opendcc/base/vendor/eventpp/eventqueue.h>
#include <pxr/usd/usd/notice.h>
#include <memory>
#include <thread>
#include <future>
#include <mutex>
#include <queue>

OPENDCC_NAMESPACE_OPEN

class StageObjectChangedWatcher;
class LayerStateDelegatesHolder;

class USD_LIVE_SHARE_API ShareEditsContext
{
public:
    struct ConnectionSettings
    {
        // GCC and clang bug https://bugs.llvm.org/show_bug.cgi?id=36684
        ConnectionSettings() {};

        std::string hostname = "127.0.0.1";
        uint32_t listener_port = 5561;
        uint32_t publisher_port = 5562;
        uint32_t content_transfer_sender_port = 5560;
        uint32_t content_transfer_receiver_port = 5559;
    };

private:
    class StageWatcher : public PXR_NS::TfWeakBase
    {
    public:
        StageWatcher(PXR_NS::UsdStageRefPtr stage, ShareEditsContext* edit_context);

    private:
        PXR_NS::TfNotice::Key m_key;
        ShareEditsContext* m_context = nullptr;
        void callback(PXR_NS::UsdNotice::ObjectsChanged const& notice, PXR_NS::UsdStageWeakPtr const& sender);
    };

    void* m_context;
    std::unique_ptr<StageWatcher> m_edit_target_watcher;
    std::shared_ptr<LayerTreeWatcher> m_layer_tree_watcher;
    std::shared_ptr<LayerStateDelegatesHolder> m_layer_state_delegates;
    LayerTreeWatcher::SublayersChangedDispatcherHandle m_sublayer_changed_key;
    uint64_t m_context_id;
    std::string m_layer_transfer_path;

    std::queue<std::unique_ptr<UsdEditBase>> m_staged_edits;
    std::mutex m_mutex;
    ConnectionSettings m_connection_settings;
    using Lock = std::lock_guard<std::mutex>;

    std::thread m_edit_share_thread;
    std::thread m_layer_transfer_thread;
    std::thread m_listener_thread;
    std::promise<void> m_stop_signal;
    std::future<void> m_future;

    enum class EventType
    {
        Work
    };
    using EventQueue = eventpp::EventQueue<EventType, void(const std::function<void()>&)>;
    EventQueue m_event_queue;

    bool m_is_processing_incoming_edits = false;

    void live_share_worker();
    void transfer_content_worker();
    void transfer_content();

public:
    ShareEditsContext(PXR_NS::UsdStageRefPtr stage, const std::string& layer_transfer_path, std::shared_ptr<LayerTreeWatcher> layer_tree_watcher,
                      std::shared_ptr<LayerStateDelegatesHolder> layer_state_delegates, const ConnectionSettings& connection_settings = {});
    ~ShareEditsContext();
    void send_edit(std::unique_ptr<UsdEditBase> edit);
    void process();
    bool is_processing_incoming_edits() const;
};
OPENDCC_NAMESPACE_CLOSE
