/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/hydra_op/ui/node_editor/api.h"
#include "opendcc/usd_editor/usd_node_editor/graph_model.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/stage_watcher.h"
#include "opendcc/usd_editor/common_cmds/parent_prim.h"
#include "opendcc/usd_editor/common_cmds/rename_prim.h"
#include <unordered_set>

OPENDCC_NAMESPACE_OPEN

class NodeProvider;
struct CallbackHandlers;
class OPENDCC_HYDRA_OP_NODE_EDITOR_API HydraOpGraphModel : public UsdGraphModel
{
    Q_OBJECT
public:
    HydraOpGraphModel(QObject* parent = nullptr);
    HydraOpGraphModel(const HydraOpGraphModel&) = delete;
    HydraOpGraphModel(HydraOpGraphModel&&) = delete;
    HydraOpGraphModel& operator=(const HydraOpGraphModel&) = delete;
    HydraOpGraphModel& operator=(HydraOpGraphModel&&) = delete;
    ~HydraOpGraphModel() override;

    std::vector<std::string> get_input_names(const NodeId& node_id) const;
    std::vector<std::string> get_output_names(const NodeId& node_id) const;
    virtual std::string get_property_name(const PortId& port_id) const override;
    PXR_NS::SdfPath to_usd_path(const PortId& node_id) const override;
    NodeId from_usd_path(const PXR_NS::SdfPath& path, const PXR_NS::SdfPath& root) const override;
    static bool can_connect(PXR_NS::SdfValueTypeName src, PXR_NS::SdfValueTypeName dst);
    virtual bool can_connect(const Port& start_port, const Port& end_port) const override;

    static bool is_supported_type_for_translate_api(const PXR_NS::TfToken& type);

    void delete_connection(const ConnectionId& connection) override;
    void remove(const QVector<NodeId>& nodes, const QVector<ConnectionId>& connections) override;
    void toggle_node_bypass(const NodeId& node_id);
    virtual bool has_port(const PortId& port) const override;

    bool is_node_bypassed(const NodeId& node_id) const;
    bool has_add_port(const PXR_NS::UsdPrim& node_prim) const;
    void set_root(const PXR_NS::SdfPath& network_path) override;
    PXR_NS::SdfPath get_root() const override;
    virtual QVector<ConnectionId> get_connections_for_node(const NodeId& node_id) const override;
    virtual QVector<ConnectionId> get_connections() const override;
    virtual QVector<NodeId> get_nodes() const override;
    virtual PXR_NS::UsdPrim get_prim_for_node(const NodeId& node_id) const override;
    virtual QPointF get_node_position(const NodeId& node_id) const override;

    void add_input(const NodeId& node_id, const std::string& port_name);
    bool set_terminal_node(const NodeId& node_id);
    bool set_bypass(const NodeId& node_id, bool value);
    NodeId get_terminal_node() const;
    void set_expansion_state(const NodeId& node, PXR_NS::TfToken expansion_state) override;
    PXR_NS::TfToken get_expansion_state(const NodeId& node) const override;
    NodeId get_node_id_from_port(const PortId& port) const override;
    virtual bool connect_ports(const Port& start_port, const Port& end_port) override;
    void try_add_prim(const PXR_NS::SdfPath& prim_path) override;
    void try_remove_prim(const PXR_NS::SdfPath& prim_path) override;
    void try_update_prop(const PXR_NS::SdfPath& prop_path) override;
    PXR_NS::UsdPrim create_usd_prim(const PXR_NS::TfToken& name, const PXR_NS::TfToken& type, const PXR_NS::SdfPath& parent_path,
                                    bool change_selection = true) override;
    bool can_be_root(const NodeId& node_id) const;
    bool can_fall_through(const NodeId& node_id) const;
    bool is_supported_prim_type(const PXR_NS::UsdPrim& prim) const;
Q_SIGNALS:
    void terminal_node_changed(std::string terminal_node_path);

public Q_SLOTS:
    virtual void on_selection_set(const QVector<std::string>& nodes, const QVector<OPENDCC_NAMESPACE::ConnectionId>& connections);

protected:
    void stage_changed_impl() override;
    virtual void on_selection_changed() override;
    virtual std::unique_ptr<MoveAction> on_node_moved(const NodeId& node_id, const QPointF& old_pos, const QPointF& new_pos) override;
    void on_rename() override;

private:
    friend class NodegraphNodeMoveAction;
    friend class CallbackHandlers;
    void init_scene_graph();
    void move_nodegraph_node(const NodeId& node_id, const QPointF& pos);

    PXR_NS::SdfPath m_root;
    PXR_NS::SdfPath m_terminal_node;
    std::unordered_map<NodeId, QPointF> m_graph_pos_cache;
    std::unique_ptr<CallbackHandlers> m_handlers;
};

OPENDCC_NAMESPACE_CLOSE
