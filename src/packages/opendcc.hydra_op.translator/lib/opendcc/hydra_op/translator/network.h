/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/hydra_op/translator/api.h"
#include <pxr/imaging/hd/sceneIndex.h>
#include "opendcc/app/core/stage_watcher.h"
#include "opendcc/hydra_op/translator/node_translator.h"
#include "opendcc/base/vendor/eventpp/callbacklist.h"
#include <memory>

PXR_NAMESPACE_OPEN_SCOPE
class UsdHydraOpNodegraph;
PXR_NAMESPACE_CLOSE_SCOPE

OPENDCC_NAMESPACE_OPEN

class HydraOpTranslatorRegistry;
class GroupNode;
class SceneIndexNode;
class HydraOpNetworkModifier;
class IHydraOpNetworkDataModifier;
class IHydraOpUSDTranslator;

template <class T>
class PassKey
{
    friend T;
    PassKey() = default;
};

struct Port
{
    PXR_NS::TfToken name;
    struct Route
    {
        PXR_NS::SdfPathVector inputs; // connection to this port
        PXR_NS::SdfPathVector outputs; // outcoming connections from this port
    };
    Route route;
};

class GraphNode
{
public:
    virtual ~GraphNode() = default;
    GraphNode(const PXR_NS::SdfPath& name, std::weak_ptr<IHydraOpUSDTranslator> translator);

    const PXR_NS::SdfPath& get_name() const;
    const std::vector<Port>& get_input_ports() const;
    const std::vector<Port>& get_output_ports() const;
    const Port* get_port_by_name(const PXR_NS::TfToken& name) const;

    PXR_NS::HdSceneIndexBaseRefPtr get_scene_index();

    bool is_bypassed() const;
    void set_bypass(bool bypass);

    GroupNode* get_group() const;

    // Convenient downcasts
    SceneIndexNode* as_si_node();

    // Topology accessors
    virtual GraphNode* resolve_to_node(const PXR_NS::SdfPath& connection_path) = 0;

    // Topology modifiers
    virtual bool add_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier> = {}) = 0;
    virtual bool remove_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier> = {}) = 0;
    virtual void finalize(IHydraOpNetworkDataModifier& modifier);

    // Time
    virtual bool is_time_dependent() const = 0;
    virtual void set_time(PXR_NS::UsdTimeCode time) {};

protected:
    enum class RouteType
    {
        Input,
        Output
    };
    bool add_input_port_route(const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type);
    bool add_output_port_route(const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type);

    bool remove_input_port_route(const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type);
    bool remove_output_port_route(const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type);

    virtual PXR_NS::HdSceneIndexBaseRefPtr get_scene_index_impl() = 0;

    GraphNode* get_node(const PXR_NS::SdfPath& node_path) const;
    PXR_NS::UsdStageWeakPtr get_stage() const;

private:
    bool add_port_route(std::vector<Port>& ports, const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type);
    bool remove_port_route(std::vector<Port>& ports, const PXR_NS::TfToken& port_name, const PXR_NS::SdfPath& connection_path, RouteType type);

    PXR_NS::SdfPath name;
    std::weak_ptr<IHydraOpUSDTranslator> m_translator;
    std::vector<Port> m_input_ports;
    std::vector<Port> m_output_ports;

    bool bypass = false;
};

class SceneIndexNode : public GraphNode
{
public:
    SceneIndexNode(const PXR_NS::SdfPath& name, std::weak_ptr<IHydraOpUSDTranslator> translator,
                   std::unique_ptr<HydraOpNodeTranslator> node_translator);

    void mark_unpopulated();
    bool is_populated() const;
    void process_args_change(const PXR_NS::UsdPrim& prim, const PXR_NS::TfTokenVector& props);
    HydraOpNodeTranslator::DirtyTypeFlags get_dirty_flags(const PXR_NS::UsdPrim& prim, const PXR_NS::TfToken& property_name) const;
    void populate(const PXR_NS::UsdPrim& prim, const std::vector<PXR_NS::HdSceneIndexBaseRefPtr>& input_indices);

    void finalize(IHydraOpNetworkDataModifier& modifier) override;
    bool add_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier> = {}) override;
    bool remove_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier> = {}) override;

    GraphNode* resolve_to_node(const PXR_NS::SdfPath& connection_path) override;

    bool is_time_dependent() const override;
    void set_time(PXR_NS::UsdTimeCode time) override;

protected:
    PXR_NS::HdSceneIndexBaseRefPtr get_scene_index_impl() override;

private:
    PXR_NS::SdfPathVector inputs;
    std::unique_ptr<HydraOpNodeTranslator> translator;
    PXR_NS::HdSceneIndexBaseRefPtr scene_index;
};
class GroupNode : public GraphNode
{
public:
    // Input port name -> node
    struct PortRoute
    {
        PXR_NS::SdfPath input; // connection to this port
        PXR_NS::SdfPathVector outputs; // outcoming connections from this port
    };
    using RoutingMap = std::unordered_map<PXR_NS::TfToken, PortRoute, PXR_NS::TfToken::HashFunctor>;

    GroupNode(const PXR_NS::SdfPath& name, std::weak_ptr<IHydraOpUSDTranslator> translator);

    void add_node(const PXR_NS::SdfPath& node_path);
    void remove_node(const PXR_NS::SdfPath& node_path);
    const PXR_NS::SdfPathVector& get_nodes() const;

    void finalize(IHydraOpNetworkDataModifier& modifier) override;
    bool add_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier> = {}) override;
    bool remove_connection(const PXR_NS::SdfPath& from, const PXR_NS::SdfPath& to, PassKey<IHydraOpNetworkDataModifier> = {}) override;
    GraphNode* resolve_to_node(const PXR_NS::SdfPath& connection_path) override;

    bool is_time_dependent() const override { return false; };

protected:
    PXR_NS::HdSceneIndexBaseRefPtr get_scene_index_impl() override;

private:
    PXR_NS::SdfPathVector nodes;
};

class OPENDCC_HYDRA_OP_TRANSLATOR_API HydraOpNetwork final
{
public:
    using Dispatcher = eventpp::CallbackList<void()>;
    using Handle = Dispatcher::Handle;

    HydraOpNetwork(const PXR_NS::UsdHydraOpNodegraph& nodegraph);

    PXR_NS::HdSceneIndexBaseRefPtr get_scene_index(const PXR_NS::SdfPath& node_path);
    Handle register_for_node(const PXR_NS::SdfPath& node_path, std::function<void()> callback);
    void unregister_for_node(const PXR_NS::SdfPath& node_path, Handle handle);
    bool has_node(const PXR_NS::SdfPath& node_path) const;
    PXR_NS::SdfPath get_root() const;
    PXR_NS::UsdStageWeakPtr get_stage() const;
    void set_time(PXR_NS::UsdTimeCode time);

private:
    std::shared_ptr<HydraOpNetworkModifier> m_translator;
    std::unique_ptr<StageObjectChangedWatcher> m_stage_watcher;
};

OPENDCC_NAMESPACE_CLOSE
