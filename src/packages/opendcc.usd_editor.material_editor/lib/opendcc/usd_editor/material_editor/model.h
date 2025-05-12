/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/usd_editor/usd_node_editor/graph_model.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/stage_watcher.h"
#include "opendcc/usd_editor/material_editor/api.h"
#include "opendcc/usd_editor/common_cmds/parent_prim.h"
#include "opendcc/usd_editor/common_cmds/rename_prim.h"
#include <unordered_set>

OPENDCC_NAMESPACE_OPEN

class NodeProvider;

class OPENDCC_MATERIAL_EDITOR_API MaterialGraphModel : public UsdGraphModel
{
    Q_OBJECT
public:
    virtual bool has_port(const PortId& port) const override;

    MaterialGraphModel(QObject* parent = nullptr);
    MaterialGraphModel(const MaterialGraphModel&) = delete;
    MaterialGraphModel(MaterialGraphModel&&) = delete;
    MaterialGraphModel& operator=(const MaterialGraphModel&) = delete;
    MaterialGraphModel& operator=(MaterialGraphModel&&) = delete;
    ~MaterialGraphModel() override;

    virtual std::string get_property_name(const PortId& port_id) const override;
    PXR_NS::SdfPath to_usd_path(const PortId& node_id) const override;
    NodeId from_usd_path(const PXR_NS::SdfPath& path, const PXR_NS::SdfPath& root) const override;
    static bool can_connect(PXR_NS::SdfValueTypeName src, PXR_NS::SdfValueTypeName dst);
    virtual bool can_connect(const Port& start_port, const Port& end_port) const override;

    void delete_connection(const ConnectionId& connection) override;
    void remove(const QVector<NodeId>& nodes, const QVector<ConnectionId>& connections) override;
    void set_show_external_nodes(bool show);
    bool show_external_nodes() const;

    void set_root(const PXR_NS::SdfPath& network_path) override;
    PXR_NS::SdfPath get_root() const override;
    virtual QVector<ConnectionId> get_connections_for_node(const NodeId& node_id) const override;
    virtual QVector<ConnectionId> get_connections() const override;
    virtual QVector<NodeId> get_nodes() const override;
    virtual PXR_NS::UsdPrim get_prim_for_node(const NodeId& node_id) const override;
    virtual QPointF get_node_position(const NodeId& node_id) const override;

    PXR_NS::SdfPath get_preview_shader() const;
    void set_preview_shader(PXR_NS::SdfPath preview_shader_path);
    void set_expansion_state(const NodeId& node, PXR_NS::TfToken expansion_state) override;
    PXR_NS::TfToken get_expansion_state(const NodeId& node) const override;
    NodeId get_node_id_from_port(const PortId& port) const;
    virtual bool connect_ports(const Port& start_port, const Port& end_port) override;
    void try_add_prim(const PXR_NS::SdfPath& prim_path) override;
    void try_remove_prim(const PXR_NS::SdfPath& prim_path) override;
    void try_update_prop(const PXR_NS::SdfPath& prop_path) override;

    bool is_external_node(const NodeId& node_id) const;
    bool can_be_root(const NodeId& node_id) const;
    bool can_fall_through(const NodeId& node_id) const;
    bool is_supported_prim_type(const PXR_NS::UsdPrim& prim) const;
Q_SIGNALS:
    void preview_shader_changed(PXR_NS::SdfPath preview_shader_path);

public Q_SLOTS:
    virtual void on_selection_set(const QVector<std::string>& nodes, const QVector<OPENDCC_NAMESPACE::ConnectionId>& connections);

protected:
    void stage_changed_impl() override;
    virtual void on_selection_changed() override;
    virtual std::unique_ptr<MoveAction> on_node_moved(const NodeId& node_id, const QPointF& old_pos, const QPointF& new_pos) override;
    void on_rename() override;

private:
    friend class MaterialNodeMoveAction;
    friend class ExternalNodeMoveAction;
    friend class MaterialOverrideUpdate;

    void init_material_network();
    void update_material_override();
    PXR_NS::VtValue build_mat_network_override() const;

    void move_material_node(bool is_input, const QPointF& pos);
    void move_external_node(const NodeId& node_id, const QPointF& pos);

    PXR_NS::SdfPath m_network_path;
    PXR_NS::SdfPath m_preview_shader;

    std::unordered_map<NodeId, QPointF> m_mat_in_pos;
    std::unordered_map<NodeId, QPointF> m_mat_out_pos;
    std::unordered_map<NodeId, QPointF> m_external_node_pos;
    bool m_show_external_nodes = false;
};

OPENDCC_NAMESPACE_CLOSE
