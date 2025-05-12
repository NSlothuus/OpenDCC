// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/translator/network.h"
#include "opendcc/hydra_op/translator/registry.h"
#include "opendcc/hydra_op/translator/node_translator.h"
#include "opendcc/hydra_op/translator/network_modifier.h"
#include "opendcc/hydra_op/schema/group.h"
#include <opendcc/base/logging/logger.h>

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    bool remove(const SdfPath& target, SdfPathVector& src)
    {
        auto rm_it = std::remove(src.begin(), src.end(), target);
        if (rm_it == src.end())
        {
            return false;
        }
        src.erase(rm_it, src.end());
        return true;
    }
    bool add_unique(const SdfPath& target, SdfPathVector& src)
    {
        if (std::find(src.begin(), src.end(), target) == src.end())
        {
            src.emplace_back(target);
            return true;
        }
        return false;
    }

    template <class TMethod, class U, class TReturn, class... TArgs>
    TReturn invoke_with_return(TMethod&& method, U&& object, TReturn&& ret, TArgs&&... args)
    {
        return object ? std::invoke(std::forward<TMethod>(method), std::forward<U>(object), std::forward<TArgs>(args)...) : ret;
    }
};

HydraOpNetwork::HydraOpNetwork(const PXR_NS::UsdHydraOpNodegraph& nodegraph)
{
    OPENDCC_ASSERT(nodegraph);
    m_translator = HydraOpNetworkModifier::create(nodegraph);
    m_stage_watcher = std::make_unique<StageObjectChangedWatcher>(
        nodegraph.GetPrim().GetStage(), [this](const UsdNotice::ObjectsChanged& notice) { m_translator->process_changes(notice); });
}

PXR_NS::HdSceneIndexBaseRefPtr HydraOpNetwork::get_scene_index(const PXR_NS::SdfPath& node_path)
{
    if (auto node = m_translator->get_node(node_path))
    {
        return node->get_scene_index();
    }
    return nullptr;
}

HydraOpNetwork::Handle HydraOpNetwork::register_for_node(const PXR_NS::SdfPath& node_path, std::function<void()> callback)
{
    return m_translator->subscribe_for_node(node_path, callback);
}

void HydraOpNetwork::unregister_for_node(const PXR_NS::SdfPath& node_path, Handle handle)
{
    return m_translator->unsubscribe_for_node(node_path, handle);
}

bool HydraOpNetwork::has_node(const PXR_NS::SdfPath& node_path) const
{
    return m_translator->get_node(node_path) != nullptr;
}

PXR_NS::SdfPath HydraOpNetwork::get_root() const
{
    return m_translator->get_root();
}

PXR_NS::UsdStageWeakPtr HydraOpNetwork::get_stage() const
{
    return m_translator->get_stage();
}

void HydraOpNetwork::set_time(PXR_NS::UsdTimeCode time)
{
    m_translator->set_time(time);
}

bool SceneIndexNode::add_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier>)
{
    if (to.GetPrimPath() == get_name())
    {
        return add_input_port_route(to.GetNameToken(), from, RouteType::Input);
    }
    else if (from.GetPrimPath() == get_name())
    {
        return add_output_port_route(from.GetNameToken(), to, RouteType::Output);
    }
    return false;
}

bool SceneIndexNode::remove_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier>)
{
    if (to.GetPrimPath() == get_name())
    {
        return remove_input_port_route(to.GetNameToken(), from, RouteType::Input);
    }
    else if (from.GetPrimPath() == get_name())
    {
        return remove_output_port_route(from.GetNameToken(), to, RouteType::Output);
    }
    return false;
}

GraphNode* SceneIndexNode::resolve_to_node(const PXR_NS::SdfPath& connection_path)
{
    return connection_path.GetPrimPath() == get_name() ? this : nullptr;
}

bool SceneIndexNode::is_time_dependent() const
{
    return translator->is_time_dependent();
}

void SceneIndexNode::set_time(PXR_NS::UsdTimeCode time)
{
    translator->on_time_changed(get_stage()->GetPrimAtPath(get_name()), scene_index, time);
}

const Port* GraphNode::get_port_by_name(const PXR_NS::TfToken& name) const
{
    const auto& inputs = get_input_ports();
    auto it = std::find_if(inputs.begin(), inputs.end(), [name](const Port& port) { return port.name == name; });
    if (it != inputs.end())
    {
        return &(*it);
    }

    const auto& outputs = get_output_ports();
    it = std::find_if(outputs.begin(), outputs.end(), [name](const Port& port) { return port.name == name; });
    if (it != outputs.end())
    {
        return &(*it);
    }

    return nullptr;
}

void SceneIndexNode::finalize(IHydraOpNetworkDataModifier& modifier)
{
    for (const auto& out_port : get_output_ports())
    {
        for (const auto& out : out_port.route.outputs)
        {
            modifier.remove_connection(get_name().AppendProperty(out_port.name), out);
        }
    }
    for (const auto& in_port : get_input_ports())
    {
        for (const auto& in : in_port.route.inputs)
        {
            modifier.remove_connection(in, get_name().AppendProperty(in_port.name));
        }
    }

    GraphNode::finalize(modifier);
}

SceneIndexNode::SceneIndexNode(const PXR_NS::SdfPath& name, std::weak_ptr<IHydraOpUSDTranslator> translator,
                               std::unique_ptr<HydraOpNodeTranslator> node_translator)
    : GraphNode(name, translator)
    , translator(std::move(node_translator))
{
}

void SceneIndexNode::mark_unpopulated()
{
    scene_index = nullptr;
}

bool SceneIndexNode::is_populated() const
{
    return scene_index != nullptr;
}

void SceneIndexNode::process_args_change(const PXR_NS::UsdPrim& prim, const PXR_NS::TfTokenVector& props)
{
    OPENDCC_ASSERT(is_populated());
    translator->process_args_change(prim, props, scene_index);
}

HydraOpNodeTranslator::DirtyTypeFlags SceneIndexNode::get_dirty_flags(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& property_name) const
{
    return translator->get_dirty_flags(prim, property_name);
}

PXR_NS::HdSceneIndexBaseRefPtr SceneIndexNode::get_scene_index_impl()
{
    if (is_populated())
    {
        return scene_index;
    }

    std::vector<HdSceneIndexBaseRefPtr> input_indices;
    for (const auto& in : get_input_ports())
    {
        for (const auto& port_input : in.route.inputs)
        {
            auto node = get_node(port_input.GetPrimPath());
            OPENDCC_ASSERT(node);
            auto resolved_node = node->resolve_to_node(port_input);
            if (!resolved_node)
            {
                continue;
            }
            if (auto in_index = resolved_node->get_scene_index())
            {
                input_indices.push_back(in_index);
            }
        }
    }
    populate(get_stage()->GetPrimAtPath(get_name()), input_indices);

    return scene_index;
}

void SceneIndexNode::populate(const PXR_NS::UsdPrim& prim, const std::vector<HdSceneIndexBaseRefPtr>& input_indices)
{
    if (is_populated())
    {
        return;
    }

    scene_index = translator->populate(prim, input_indices);
}

GroupNode::GroupNode(const PXR_NS::SdfPath& name, std::weak_ptr<IHydraOpUSDTranslator> translator)
    : GraphNode(name, translator)
{
}

bool GroupNode::add_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier>)
{
    OPENDCC_ASSERT(from.IsPropertyPath());
    OPENDCC_ASSERT(to.IsPropertyPath());

    if (from.GetPrimPath() == get_name())
    {
        // if output connection from Group node, check that we don't connect to nodes inside this group
        if (from.GetNameToken() == UsdHydraOpTokens->outputsOut)
        {
            if (to.GetPrimPath().GetParentPath() != get_name())
            {
                return add_output_port_route(UsdHydraOpTokens->outputsOut, to, RouteType::Output);
            }
            return false;
        }

        return add_input_port_route(from.GetNameToken(), to, RouteType::Output);
    }
    else if (to.GetPrimPath() == get_name())
    {
        if (to.GetNameToken() == UsdHydraOpTokens->outputsOut)
        {
            add_output_port_route(UsdHydraOpTokens->outputsOut, from, RouteType::Input);
        }
        else
        {
            add_input_port_route(to.GetNameToken(), from, RouteType::Input);
        }
        return true;
    }
    return false;
}

bool GroupNode::remove_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier>)
{
    if (from.GetPrimPath() == get_name())
    {
        const auto to_parent_path = to.GetPrimPath().GetParentPath();

        if (from.GetNameToken() == UsdHydraOpTokens->outputsOut)
        {
            if (to_parent_path != get_name())
            {
                return remove_output_port_route(UsdHydraOpTokens->outputsOut, to, RouteType::Output);
            }
            return false;
        }

        // attempt to connect outside of group boundaries
        if (to_parent_path != get_name())
        {
            return false;
        }

        return remove_input_port_route(from.GetNameToken(), from, RouteType::Input);
    }
    else if (to.GetPrimPath() == get_name())
    {
        if (to.GetNameToken() == UsdHydraOpTokens->outputsOut)
        {
            return remove_output_port_route(UsdHydraOpTokens->outputsOut, from, RouteType::Input);
        }
        else
        {
            return remove_input_port_route(to.GetNameToken(), from, RouteType::Input);
        }
    }
    return false;
}

GraphNode* GroupNode::resolve_to_node(const PXR_NS::SdfPath& connection_path)
{
    const auto port_name = connection_path.GetNameToken();
    const auto node_name = connection_path.GetPrimPath();
    if (node_name == get_name() && port_name == UsdHydraOpTokens->outputsOut)
    {
        return this;
    }

    if (const auto port = get_port_by_name(port_name))
    {
        if (port->name == UsdHydraOpTokens->outputsOut)
        {
            return this;
        }

        if (!port->route.inputs.empty())
        {
            const auto& first_input = port->route.inputs.front();
            const auto node = get_node(first_input.GetPrimPath());
            return node->resolve_to_node(first_input);
        }
    }

    return nullptr;
}

PXR_NS::HdSceneIndexBaseRefPtr GroupNode::get_scene_index_impl()
{
    const auto& output_ports = get_output_ports();
    if (output_ports.empty())
    {
        return nullptr;
    }

    auto& output_route = output_ports.front().route;
    if (!output_route.inputs.empty())
    {
        const auto& first = output_route.inputs[0];
        auto node = get_node(first.GetPrimPath());
        OPENDCC_ASSERT(node);
        return node->get_scene_index();
    }
    return nullptr;
}

void GroupNode::add_node(const PXR_NS::SdfPath& node_path)
{
    add_unique(node_path, nodes);
}

void GroupNode::remove_node(const PXR_NS::SdfPath& node_path)
{
    remove(node_path, nodes);
}

const PXR_NS::SdfPathVector& GroupNode::get_nodes() const
{
    return nodes;
}

void GroupNode::finalize(IHydraOpNetworkDataModifier& modifier)
{
    const auto nodes = get_nodes();
    for (const auto& n : nodes)
    {
        get_node(n)->finalize(modifier);
    }

    if (auto output = get_port_by_name(UsdHydraOpTokens->outputsOut))
    {
        for (const auto& out : output->route.outputs)
        {
            modifier.remove_connection(get_name().AppendProperty(UsdHydraOpTokens->outputsOut), out);
        }
    }

    for (const auto& in_port : get_input_ports())
    {
        for (const auto& in : in_port.route.inputs)
        {
            modifier.remove_connection(in, get_name().AppendProperty(in_port.name));
        }
    }

    GraphNode::finalize(modifier);
}

GraphNode::GraphNode(const PXR_NS::SdfPath& name, std::weak_ptr<IHydraOpUSDTranslator> translator)
    : name(name)
    , m_translator(translator)
{
    if (auto group = get_group())
    {
        group->add_node(get_name());
    }
}

void GraphNode::set_bypass(bool bypass)
{
    this->bypass = bypass;
}

const PXR_NS::SdfPath& GraphNode::get_name() const
{
    return name;
}

const std::vector<Port>& GraphNode::get_input_ports() const
{
    return m_input_ports;
}

const std::vector<Port>& GraphNode::get_output_ports() const
{
    return m_output_ports;
}

GroupNode* GraphNode::get_group() const
{
    const auto grp_path = get_name().GetParentPath();
    OPENDCC_ASSERT(grp_path != get_name());

    return dynamic_cast<GroupNode*>(get_node(grp_path));
}

bool GraphNode::is_bypassed() const
{
    return bypass;
}

SceneIndexNode* GraphNode::as_si_node()
{
    return dynamic_cast<SceneIndexNode*>(this);
}

PXR_NS::HdSceneIndexBaseRefPtr GraphNode::get_scene_index()
{
    if (is_bypassed())
    {
        for (const auto& in_port : get_input_ports())
        {
            for (const auto& in : in_port.route.inputs)
            {
                auto node = get_node(in.GetPrimPath());
                if (auto resolved_node = node->resolve_to_node(in))
                {
                    return resolved_node->get_scene_index();
                }
                else
                {
                    return nullptr;
                }
            }
        }
        return nullptr;
    }

    return get_scene_index_impl();
}

void GraphNode::finalize(IHydraOpNetworkDataModifier& modifier)
{
    if (auto grp = get_group())
    {
        grp->remove_node(get_name());
    }

    modifier.mark_finalized(get_name());
}

bool GraphNode::add_input_port_route(const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type)
{
    return add_port_route(m_input_ports, port_name, connection_path, type);
}

bool GraphNode::add_output_port_route(const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type)
{
    return add_port_route(m_output_ports, port_name, connection_path, type);
}

bool GraphNode::remove_input_port_route(const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type)
{
    return remove_port_route(m_input_ports, port_name, connection_path, type);
}

bool GraphNode::remove_output_port_route(const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type)
{
    return remove_port_route(m_output_ports, port_name, connection_path, type);
}

GraphNode* GraphNode::get_node(const PXR_NS::SdfPath& node_path) const
{
    OPENDCC_ASSERT(node_path.IsAbsoluteRootOrPrimPath());
    return m_translator.lock()->get_node(node_path);
}

PXR_NS::UsdStageWeakPtr GraphNode::get_stage() const
{
    return m_translator.lock()->get_stage();
}

bool GraphNode::add_port_route(std::vector<Port>& ports, const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type)
{
    OPENDCC_ASSERT(!port_name.IsEmpty());
    auto port_it = std::find_if(ports.begin(), ports.end(), [&port_name](const Port& port) { return port.name == port_name; });

    if (port_it == ports.end())
    {
        Port new_port;
        new_port.name = port_name;

        if (type == RouteType::Input)
            new_port.route.inputs.push_back(connection_path);
        else
            new_port.route.outputs.push_back(connection_path);
        ports.emplace_back(std::move(new_port));
        return true;
    }

    auto& connections = type == RouteType::Input ? port_it->route.inputs : port_it->route.outputs;

    if (std::find(connections.begin(), connections.end(), connection_path) == connections.end())
    {
        connections.push_back(connection_path);
        return true;
    }

    return false;
}

bool GraphNode::remove_port_route(std::vector<Port>& ports, const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type)
{
    OPENDCC_ASSERT(!port_name.IsEmpty());

    auto port_it = std::find_if(ports.begin(), ports.end(), [&port_name](const Port& port) { return port.name == port_name; });

    if (port_it != ports.end())
    {
        auto& connections = type == RouteType::Input ? port_it->route.inputs : port_it->route.outputs;
        return remove(connection_path, connections);
    }

    return false;
}

OPENDCC_NAMESPACE_CLOSE
