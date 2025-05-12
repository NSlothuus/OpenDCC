/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/hydra_op/api.h"
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include "opendcc/app/core/application.h"

OPENDCC_NAMESPACE_OPEN

class HydraOpTerminalSceneIndex;
class ViewNodeHandler;
class HydraOpNetworkRegistry;
class HydraOpNetwork;

class OPENDCC_HYDRA_OP_API HydraOpSession final
{
public:
    enum class EventType
    {
        ViewNodeChanged,
        SelectionChanged
    };

    using Dispatcher = eventpp::EventDispatcher<EventType, void()>;
    using Handle = Dispatcher::Handle;

    ~HydraOpSession();
    static HydraOpSession& instance();

    void set_view_node(const PXR_NS::SdfPath& node_path);
    const PXR_NS::SdfPath& get_view_node() const;

    std::shared_ptr<HydraOpNetwork> get_view_node_network() const;
    PXR_NS::TfRefPtr<HydraOpTerminalSceneIndex> get_view_scene_index() const;

    HydraOpNetworkRegistry& get_network_registry() const;

    Handle register_event_handler(EventType event_type, const std::function<void()>& callback);
    void unregister_event_handler(EventType event_type, const Handle& handle);

    SelectionList get_selection() const;
    void set_selection(const SelectionList& new_selection);

private:
    HydraOpSession();
    HydraOpSession(const HydraOpSession&) = delete;
    HydraOpSession(HydraOpSession&&) = delete;
    HydraOpSession& operator=(const HydraOpSession&) = delete;
    HydraOpSession& operator=(HydraOpSession&&) = delete;

    std::unique_ptr<ViewNodeHandler> m_view_node_handler;
    std::unique_ptr<HydraOpNetworkRegistry> m_network_registry;
    Dispatcher m_dispatcher;
    Application::CallbackHandle m_current_stage_changed_cid;
    Application::CallbackHandle m_time_changed_cid;
    SelectionList m_selection;
    PXR_NS::HdContainerDataSourceHandle m_hd_data_source;
};

OPENDCC_NAMESPACE_CLOSE
