/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include "opendcc/ui/node_editor/graph_model.h"
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

class LiveConnectionItem;
class NodeEditorItemRegistry;
class NodeItem;
class ConnectionItem;
class NodeEditorView;
class ThumbnailCache;

extern OPENDCC_UI_NODE_EDITOR_API const qreal s_space_for_insert;
extern OPENDCC_UI_NODE_EDITOR_API const qreal s_pos_offset_for_insert;

QPointF OPENDCC_UI_NODE_EDITOR_API to_scene_position(const QPointF& model_pos, int node_width);
QPointF OPENDCC_UI_NODE_EDITOR_API to_model_position(const QPointF& scene_pos, int node_width);

class OPENDCC_UI_NODE_EDITOR_API NodeEditorScene : public QGraphicsScene
{
    Q_OBJECT
public:
    enum class GraphicsItemType
    {
        Node = QGraphicsItem::UserType + 1,
        Connection = QGraphicsItem::UserType + 2,
        Port = QGraphicsItem::UserType + 3,
        Group = QGraphicsItem::UserType + 4,
    };
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;

    NodeEditorScene(GraphModel& graph_model, NodeEditorItemRegistry& item_registry, QObject* parent = nullptr);
    virtual ~NodeEditorScene();

    const GraphModel& get_model() const { return *m_graph_model; }

    void set_model(GraphModel& graph_model);
    void set_item_registry(NodeEditorItemRegistry& item_registry);

    void initialize();

    void add_node_item(const NodeId& node_id);
    void add_connection_item(const ConnectionId& connection_id);

    void remove_node_item(const NodeId& node);
    void remove_connection_item(const ConnectionId& connection);

    void set_grabber_item(QGraphicsItem* item);
    QGraphicsItem* get_grabber_item() const;
    void remove_grabber_item();
    bool has_grabber_item() const;

    QVector<NodeItem*> get_node_items() const;
    QVector<NodeItem*> get_selected_node_items() const;
    NodeItem* get_item_for_node(const NodeId& node_id) const;
    ConnectionItem* get_item_for_connection(const ConnectionId& connection) const;
    QVector<ConnectionItem*> get_connection_items_for_node(const NodeId& node_id) const;
    ConnectionItem* get_connection_item(const QPointF& pos) const;
    NodeEditorView* get_view() const;

    void update_node(const NodeId& node_id);
    void update_color(const NodeId& node_id);
    void update_port(const PortId& port_id);
    void set_selection(const QVector<NodeId>& nodes, const QVector<ConnectionId>& connections);
    void get_selection(QVector<NodeId>& nodes, QVector<ConnectionId>& connections) const;

    void set_thumbnail_cache(ThumbnailCache* cache);
    ThumbnailCache* get_thumbnail_cache() const;

    void begin_move(const QVector<NodeId>& nodes);
    void end_move();

    void begin_resize(const NodeId& node);
    void end_resize();

    QList<QGraphicsItem*> get_items_from_snapping_rect(const QPointF& center_pos, const qreal& center_pos_offset, const qreal& side_length) const;
Q_SIGNALS:
    void node_renamed(const std::string& old_name, const std::string& new_name);
    void nodes_moved(const QVector<std::string>& node_ids, const QVector<QPointF>& old_pos, const QVector<QPointF>& new_pos);
    void node_resized(const std::string& node_id, const float old_width, const float old_height, const float new_width, const float new_height);
    void node_double_clicked(const std::string& node_id);
    void node_hovered(const std::string& node_id, bool is_hover = true);

    void connection_removed(const ConnectionId& connection_id);
    void connection_double_clicked(const ConnectionId& connection_id);
    void connection_hovered(const ConnectionId& connection_id, bool is_hover = true);

    void port_pressed(const Port& port);
    void port_released(const Port& port);
    void port_hovered(const Port& port, bool is_hover = true);
    void port_need_tool_tip(const Port& port);

    void group_hovered(const std::string& name, bool is_hover = true);
    void group_need_tool_tip(const std::string& name);

    void selection_changed(const QVector<std::string>& nodes, const QVector<OPENDCC_NAMESPACE::ConnectionId>& connections);

protected:
    NodeEditorItemRegistry& get_item_registry() { return *m_item_registry; }
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    NodeItem* make_node(const NodeId& node_id);
    void on_scene_selection_changed();

    std::unordered_map<NodeId, NodeItem*> m_nodes;
    std::unordered_map<ConnectionId, ConnectionItem*, ConnectionId::Hash> m_connections;

    std::unordered_map<NodeItem*, QPointF> m_move_items_cache;
    std::pair<NodeItem*, QRectF> m_resize_cache;
    GraphModel* m_graph_model = nullptr;
    NodeEditorItemRegistry* m_item_registry = nullptr;
    QGraphicsItem* m_grabber_item = nullptr;
    ThumbnailCache* m_thumbnail_cache = nullptr;
    bool m_updating_selection = false;
};
OPENDCC_NAMESPACE_CLOSE
