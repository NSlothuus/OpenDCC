// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/session.h"
#include "opendcc/hydra_op/translator/terminal_scene_index.h"
#include "opendcc/hydra_op/translator/network_registry.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

class ViewNodeHandler
{
public:
    ViewNodeHandler(const HydraOpSession& session)
        : m_session(session)
    {
        m_scene_index = HydraOpTerminalSceneIndex::New(nullptr);
    }
    ~ViewNodeHandler()
    {
        if (m_network)
        {
            m_network->unregister_for_node(m_path, m_dirty_view_node_cid);
        }
    }

    bool set_view_node(const SdfPath& new_view_node_path)
    {
        if (new_view_node_path == m_path)
        {
            return false;
        }

        if (m_network)
        {
            m_network->unregister_for_node(m_path, m_dirty_view_node_cid);
        }

        auto network = m_session.get_network_registry().request_network(new_view_node_path);
        if (network != m_network)
        {
            m_network = network;
        }
        m_path = new_view_node_path;

        if (m_network)
        {
            m_network->set_time(Application::instance().get_current_time());
            m_dirty_view_node_cid = m_network->register_for_node(m_path, [this] { m_scene_index->reset_index(m_network->get_scene_index(m_path)); });
            m_scene_index->reset_index(m_network->get_scene_index(m_path));
        }
        else
        {
            m_scene_index->reset_index(nullptr);
        }

        return true;
    }

    const SdfPath& get_view_node() const { return m_path; }

    TfRefPtr<HydraOpTerminalSceneIndex> get_scene_index() const { return m_scene_index; }

private:
    const HydraOpSession& m_session;
    PXR_NS::SdfPath m_path;
    std::shared_ptr<HydraOpNetwork> m_network;
    PXR_NS::TfRefPtr<HydraOpTerminalSceneIndex> m_scene_index;
    HydraOpNetwork::Handle m_dirty_view_node_cid;
};

HydraOpSession::HydraOpSession()
{
    m_current_stage_changed_cid = Application::instance().register_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, [this] {
        m_network_registry.reset(new HydraOpNetworkRegistry(Application::instance().get_session()->get_current_stage()));

        set_selection(SelectionList());
        set_view_node(SdfPath::EmptyPath());
    });
    m_time_changed_cid = Application::instance().register_event_callback(Application::EventType::CURRENT_TIME_CHANGED, [this] {
        if (auto view_network = get_view_node_network())
        {
            view_network->set_time(Application::instance().get_current_time());
        }
    });
    m_network_registry.reset(new HydraOpNetworkRegistry(Application::instance().get_session()->get_current_stage()));
    m_view_node_handler = std::make_unique<ViewNodeHandler>(*this);
}

HydraOpSession::~HydraOpSession()
{
    Application::instance().unregister_event_callback(Application::EventType::CURRENT_STAGE_CHANGED, m_current_stage_changed_cid);
    Application::instance().unregister_event_callback(Application::EventType::CURRENT_TIME_CHANGED, m_time_changed_cid);
}

HydraOpSession& HydraOpSession::instance()
{
    static HydraOpSession instance;
    return instance;
}

void HydraOpSession::set_view_node(const PXR_NS::SdfPath& node_path)
{
    if (m_view_node_handler->set_view_node(node_path))
    {
        m_dispatcher.dispatch(EventType::ViewNodeChanged);
    }
}

const PXR_NS::SdfPath& HydraOpSession::get_view_node() const
{
    return m_view_node_handler->get_view_node();
}

std::shared_ptr<HydraOpNetwork> HydraOpSession::get_view_node_network() const
{
    return get_network_registry().request_network(get_view_node());
}

PXR_NS::TfRefPtr<HydraOpTerminalSceneIndex> HydraOpSession::get_view_scene_index() const
{
    return m_view_node_handler->get_scene_index();
}

HydraOpNetworkRegistry& HydraOpSession::get_network_registry() const
{
    return *m_network_registry;
}

HydraOpSession::Handle HydraOpSession::register_event_handler(EventType event_type, const std::function<void()>& callback)
{
    return m_dispatcher.appendListener(event_type, callback);
}

void HydraOpSession::unregister_event_handler(EventType event_type, const Handle& handle)
{
    m_dispatcher.removeListener(event_type, handle);
}

SelectionList HydraOpSession::get_selection() const
{
    return m_selection;
}

void HydraOpSession::set_selection(const SelectionList& new_selection)
{
    if (m_selection != new_selection)
    {
        m_selection = new_selection;
        m_dispatcher.dispatch(EventType::SelectionChanged);
    }
}

OPENDCC_NAMESPACE_CLOSE
