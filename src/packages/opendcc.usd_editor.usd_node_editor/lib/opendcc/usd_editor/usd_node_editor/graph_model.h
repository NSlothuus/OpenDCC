/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/base/qt_python.h"
#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/usd_node_editor/api.h"
#include "opendcc/usd_editor/usd_node_editor/node_provider.h"
#include "opendcc/ui/node_editor/graph_model.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/stage_watcher.h"
#include "opendcc/usd_editor/usd_node_editor/node_provider.h"
#include <pxr/usd/usd/stage.h>
#include <unordered_set>

OPENDCC_NAMESPACE_OPEN

class MoveAction
{
public:
    virtual ~MoveAction() {};
    virtual void undo() = 0;
    virtual void redo() = 0;
};

class NodeProvider;

struct GraphCache
{
    std::unordered_set<NodeId> nodes;
    std::unordered_set<ConnectionId, ConnectionId::Hash> connections;
};

class OPENDCC_USD_NODE_EDITOR_API UsdGraphModel : public GraphModel
{
    Q_OBJECT
public:
    UsdGraphModel(QObject* parent = nullptr);
    UsdGraphModel(const UsdGraphModel&) = delete;
    UsdGraphModel(UsdGraphModel&&) = delete;
    ~UsdGraphModel() override;

    virtual PXR_NS::UsdPrim get_prim_for_node(const NodeId& node_id) const;
    virtual std::string get_property_name(const PortId& port_id) const;
    virtual PXR_NS::SdfPath to_usd_path(const PortId& node_id) const = 0;
    virtual NodeId from_usd_path(const PXR_NS::SdfPath& path, const PXR_NS::SdfPath& root) const = 0;
    virtual QPointF get_node_position(const NodeId& node_id) const override;
    virtual bool can_rename(const NodeId& old_name, const NodeId& new_name) const override;
    virtual bool rename(const NodeId& old_name, const NodeId& new_name) const override;
    virtual PXR_NS::UsdPrim create_usd_prim(const PXR_NS::TfToken& name, const PXR_NS::TfToken& type, const PXR_NS::SdfPath& parent_path,
                                            bool change_selection = true);

    virtual void set_root(const PXR_NS::SdfPath& path) = 0;
    virtual PXR_NS::SdfPath get_root() const = 0;

    virtual bool can_connect(const Port& start_port, const Port& end_port) const;
    static NodeId get_node_path(const PortId& port_id);
    static NodeId get_parent_path(const PortId& port_id);

    virtual bool connect_ports(const Port& start_port, const Port& end_port) override;
    virtual PXR_NS::TfToken get_expansion_state(const NodeId& node) const = 0;
    virtual void set_expansion_state(const NodeId& node, PXR_NS::TfToken expansion_state) = 0;
    virtual bool has_port(const PortId& port) const override;

    void block_usd_notifications(bool block);

    UsdGraphModel& operator=(const UsdGraphModel&) = delete;
    UsdGraphModel& operator=(UsdGraphModel&&) = delete;

    virtual void stage_changed_impl() = 0;
    virtual void try_add_prim(const PXR_NS::SdfPath& prim_path) = 0;
    virtual void try_remove_prim(const PXR_NS::SdfPath& prim_path) = 0;
    virtual void try_update_prop(const PXR_NS::SdfPath& prop_path) = 0;
    virtual void on_rename() = 0;

    std::vector<ConnectionId> get_connections_for_prim(const PXR_NS::UsdPrim& prim) const;
    PXR_NS::UsdStageRefPtr get_stage() const;

public Q_SLOTS:
    virtual void on_nodes_moved(const QVector<std::string>& node_ids, const QVector<QPointF>& old_pos, const QVector<QPointF>& new_pos);
    virtual void on_node_resized(const std::string& node_id, const float old_width, const float old_height, const float new_width,
                                 const float new_height);
    virtual void on_selection_set(const QVector<std::string>& nodes, const QVector<OPENDCC_NAMESPACE::ConnectionId>& connections);

protected:
    virtual void on_selection_changed() = 0;
    virtual std::unique_ptr<MoveAction> on_node_moved(const NodeId& node_id, const QPointF& old_pos, const QPointF& new_pos);

    NodeProvider& get_node_provider() const { return *m_node_provider.get(); }
    void set_node_provider(std::unique_ptr<NodeProvider> node_provider) { m_node_provider = std::move(node_provider); }

    GraphCache& get_graph_cache() const { return m_graph_cache; }

private:
    Application::CallbackHandle m_selection_changed_cid;
    std::unique_ptr<NodeProvider> m_node_provider;
    mutable GraphCache m_graph_cache;
    bool m_updating_selection = false;
};

class OPENDCC_USD_NODE_EDITOR_API UsdEditorGraphModel : public UsdGraphModel
{
    Q_OBJECT
public:
    virtual void on_rename() override;
    virtual PXR_NS::SdfPath get_root() const override;

    virtual void set_root(const PXR_NS::SdfPath& path) override;

    virtual NodeId from_usd_path(const PXR_NS::SdfPath& path, const PXR_NS::SdfPath& root) const override;

    virtual PXR_NS::SdfPath to_usd_path(const PortId& node_id) const override;

    UsdEditorGraphModel(QObject* parent = nullptr);
    UsdEditorGraphModel(const UsdEditorGraphModel&) = delete;
    UsdEditorGraphModel(UsdEditorGraphModel&&) = delete;
    UsdEditorGraphModel& operator=(const UsdEditorGraphModel&) = delete;
    UsdEditorGraphModel& operator=(UsdEditorGraphModel&&) = delete;

    void add_node_to_graph(const PXR_NS::SdfPath& path);
    void remove_node_from_graph(const PXR_NS::SdfPath& path);

    QVector<ConnectionId> get_connections_for_node(const NodeId& node_id) const override;
    QVector<NodeId> get_nodes() const override;
    void delete_connection(const ConnectionId& connection) override;
    void remove(const QVector<NodeId>& nodes, const QVector<ConnectionId>& connections) override;
    QVector<ConnectionId> get_connections() const override;
    PXR_NS::TfToken get_expansion_state(const NodeId& node) const override;
    void set_expansion_state(const NodeId& node, PXR_NS::TfToken expansion_state) override;
    NodeId get_node_id_from_port(const PortId& port) const override;
    virtual void try_add_prim(const PXR_NS::SdfPath& prim_path) override;
    virtual void try_remove_prim(const PXR_NS::SdfPath& prim_path) override;
    virtual void try_update_prop(const PXR_NS::SdfPath& prop_path) override;

protected:
    void stage_changed_impl() override;
    virtual void on_selection_changed() override;

private:
    std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash> m_nodes;
    std::unordered_map<NodeId, PXR_NS::TfToken> m_expansion_state_cache;
    std::unordered_set<ConnectionId, ConnectionId::Hash> m_connections_cache;
};

OPENDCC_NAMESPACE_CLOSE
