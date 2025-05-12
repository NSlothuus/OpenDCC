// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/node_editor/scene.h"
#include "opendcc/ui/node_editor/connection.h"
#include "opendcc/ui/node_editor/view.h"
#include "opendcc/ui/node_editor/node.h"
#include "opendcc/ui/node_editor/graph_model.h"
#include "opendcc/ui/node_editor/item_registry.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QMenu>
#include <QGraphicsItem>
#include <QGraphicsScene>

OPENDCC_NAMESPACE_OPEN
const qreal s_space_for_insert = 47;
const qreal s_pos_offset_for_insert = (s_space_for_insert - 1) / 2;

QPointF to_scene_position(const QPointF& model_pos, int node_width)
{
    return QPointF(model_pos.x() * node_width, model_pos.y() * node_width);
}

QPointF to_model_position(const QPointF& scene_pos, int node_width)
{
    return QPointF(scene_pos.x() / node_width, scene_pos.y() / node_width);
}

void NodeEditorScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->acceptProposedAction();
}

NodeEditorScene::NodeEditorScene(GraphModel& graph_model, NodeEditorItemRegistry& item_registry, QObject* parent)
    : QGraphicsScene(parent)
    , m_graph_model(&graph_model)
    , m_item_registry(&item_registry)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);
    connect(this, &QGraphicsScene::selectionChanged, this, [this] {
        if (!m_updating_selection)
            on_scene_selection_changed();
    });
    initialize();
}

NodeEditorScene::~NodeEditorScene()
{
    delete m_grabber_item;
    clear();
}

void NodeEditorScene::remove_grabber_item()
{
    if (!m_grabber_item)
        return;

    removeItem(m_grabber_item);
    delete m_grabber_item;
    m_grabber_item = nullptr;
}

bool NodeEditorScene::has_grabber_item() const
{
    return m_grabber_item;
}

QVector<NodeItem*> NodeEditorScene::get_node_items() const
{
    QVector<NodeItem*> result;
    result.reserve(m_nodes.size());
    for (const auto& node : m_nodes)
        result.push_back(node.second);
    return std::move(result);
}

QVector<NodeItem*> NodeEditorScene::get_selected_node_items() const
{
    QVector<NodeItem*> result;
    for (const auto item : selectedItems())
    {
        if (auto node_item = qgraphicsitem_cast<NodeItem*>(item))
            result.push_back(node_item);
    }

    return result;
}

ConnectionItem* NodeEditorScene::get_item_for_connection(const ConnectionId& connection_id) const
{
    auto connection_iter = m_connections.find(connection_id);
    return connection_iter != m_connections.end() ? connection_iter->second : nullptr;
}

QVector<ConnectionItem*> NodeEditorScene::get_connection_items_for_node(const NodeId& node_id) const
{
    const auto connections = m_graph_model->get_connections_for_node(node_id);
    QVector<ConnectionItem*> result;
    result.reserve(connections.size());
    for (const auto& connection : connections)
    {
        auto it = m_connections.find(connection);
        if (it != m_connections.end())
            result.push_back(it->second);
    }
    return std::move(result);
}

ConnectionItem* NodeEditorScene::get_connection_item(const QPointF& pos) const
{
    auto around_cursor_items = get_items_from_snapping_rect(pos, s_pos_offset_for_insert, s_space_for_insert);
    if (around_cursor_items.empty())
    {
        return nullptr;
    }

    BasicConnectionItem* connection = nullptr;
    QVector<BasicConnectionItem*> connection_item_around_cursor;
    for (auto item : around_cursor_items)
    {
        if (auto connection_item = dynamic_cast<BasicConnectionItem*>(item))
        {
            connection_item_around_cursor.push_back(connection_item);
        }
    }
    if (connection_item_around_cursor.empty())
    {
        return nullptr;
    }

    if (connection_item_around_cursor.size() > 1)
    {
        auto step_pos = pos;
        auto is_connection_point_at_step_pos_lambda = [this](const QPointF& step_pos) -> BasicConnectionItem* {
            BasicConnectionItem* connection = nullptr;
            for (auto item : items(step_pos))
            {
                if (connection = qgraphicsitem_cast<BasicConnectionItem*>(item))
                {
                    break;
                }
            }
            return connection;
        };

        if (connection = is_connection_point_at_step_pos_lambda(step_pos))
        {
            return connection;
        }

        const QPointF move_directions[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
        const QPointF offset_point = QPointF(1, 1);
        for (int circle_num = 1, steps_in_direction_count = 2; circle_num <= s_pos_offset_for_insert; circle_num++, steps_in_direction_count += 2)
        {
            step_pos = pos - offset_point;
            for (const auto& direction : move_directions)
            {
                for (int step_num = 0; step_num < steps_in_direction_count; step_num++)
                {
                    if (connection = is_connection_point_at_step_pos_lambda(step_pos))
                    {
                        return connection;
                    }
                    step_pos += direction;
                }
            }
        }
    }
    else
    {
        connection = connection_item_around_cursor.back();
    }
    return connection;
}

NodeEditorView* NodeEditorScene::get_view() const
{
    for (const auto view : views())
    {
        if (auto node_editor_view = dynamic_cast<NodeEditorView*>(views()[0]))
            return node_editor_view;
    }
    return nullptr;
}

NodeItem* NodeEditorScene::make_node(const NodeId& node_id)
{
    if (m_nodes.find(node_id) != m_nodes.end())
        return nullptr;
    if (auto node_item = m_item_registry->make_node(*this, node_id))
    {
        m_nodes[node_id] = node_item;
        addItem(node_item);
        return node_item;
    }
    return nullptr;
}

void NodeEditorScene::on_scene_selection_changed()
{
    QVector<NodeId> nodes;
    QVector<ConnectionId> connections;
    get_selection(nodes, connections);
    Q_EMIT selection_changed(std::move(nodes), std::move(connections));
}

void NodeEditorScene::set_selection(const QVector<NodeId>& nodes, const QVector<ConnectionId>& connections)
{
    m_updating_selection = true;
    std::unordered_set<NodeId> nodes_to_select(nodes.begin(), nodes.end());
    std::unordered_set<ConnectionId, ConnectionId::Hash> connections_to_select(connections.begin(), connections.end());

    bool selection_changed = false;
    auto select_item = [this, &selection_changed](QGraphicsItem* item, bool select) {
        if (item->isSelected() != select)
        {
            item->setSelected(select);
            selection_changed = true;
        }
    };
    for (const auto& node : m_nodes)
    {
        const auto& node_id = node.first;
        const auto item = node.second;
        const auto select = nodes_to_select.find(node_id) != nodes_to_select.end();
        select_item(item, select);
    }
    for (const auto& connection : m_connections)
    {
        const auto& conn_id = connection.first;
        const auto item = connection.second;
        const auto select = connections_to_select.find(conn_id) != connections_to_select.end();
        select_item(item, select);
    }
    if (selection_changed)
        Q_EMIT selectionChanged();
    m_updating_selection = false;
}

NodeItem* NodeEditorScene::get_item_for_node(const NodeId& node_id) const
{
    auto node_iter = m_nodes.find(node_id);
    return node_iter != m_nodes.end() ? node_iter->second : nullptr;
}

void NodeEditorScene::set_model(GraphModel& graph_model)
{
    m_graph_model = &graph_model;
}

void NodeEditorScene::set_item_registry(NodeEditorItemRegistry& item_registry)
{
    m_item_registry = &item_registry;
}

void NodeEditorScene::initialize()
{
    remove_grabber_item();
    clear();
    m_nodes.clear();
    m_connections.clear();
    m_move_items_cache.clear();

    std::vector<NodeItem*> nodes;
    for (const auto& id : m_graph_model->get_nodes())
    {
        if (auto item = make_node(id))
            nodes.push_back(item);
    }
    for (const auto& id : m_graph_model->get_connections())
        add_connection_item(id);
    for (auto node : nodes)
        node->update_node();
}

void NodeEditorScene::add_node_item(const NodeId& node_id)
{
    if (auto node = make_node(node_id))
        node->update_node();
}

void NodeEditorScene::add_connection_item(const ConnectionId& connection_id)
{
    if (m_connections.find(connection_id) != m_connections.end())
        return;
    const auto start_node = get_item_for_node(m_graph_model->get_node_id_from_port(connection_id.start_port));
    const auto end_node = get_item_for_node(m_graph_model->get_node_id_from_port(connection_id.end_port));
    if (!start_node || !end_node)
        return;
    if (auto connection_item = m_item_registry->make_connection(*this, connection_id))
    {
        m_connections[connection_id] = connection_item;
        addItem(connection_item);
        start_node->add_connection(connection_item);
        end_node->add_connection(connection_item);
    }
}

void NodeEditorScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // if (has_live_connection())
    // sendEvent(m_live_connection, event);
    QGraphicsScene::mousePressEvent(event);
}

void NodeEditorScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (has_grabber_item())
    {
        m_grabber_item->ungrabMouse();
        // if (event->buttons())
        {
            QGraphicsSceneMouseEvent proxy_event(event->type());
            proxy_event.setWidget(event->widget());
            proxy_event.setScenePos(event->scenePos());
            proxy_event.setScreenPos(event->screenPos());
            proxy_event.setLastScenePos(event->lastScenePos());
            proxy_event.setLastScreenPos(event->lastScreenPos());
            proxy_event.setButton(Qt::NoButton);
            proxy_event.setButtons(Qt::NoButton);
            proxy_event.setModifiers(event->modifiers());
            proxy_event.setSource(event->source());
            proxy_event.setFlags(event->flags());
            QGraphicsScene::mouseMoveEvent(&proxy_event);
        }
        m_grabber_item->grabMouse();
        // sendEvent(m_live_connection, event);
    }
    QGraphicsScene::mouseMoveEvent(event);
}
void NodeEditorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsScene::mouseReleaseEvent(event);
}

void NodeEditorScene::set_grabber_item(QGraphicsItem* item)
{
    if (!item)
    {
        remove_grabber_item();
        return;
    }

    if (has_grabber_item())
        return;

    m_grabber_item = item;
    addItem(m_grabber_item);
    // if (auto grabber = mouseGrabberItem())
    // grabber->ungrabMouse();
    m_grabber_item->grabMouse();
}

QGraphicsItem* NodeEditorScene::get_grabber_item() const
{
    return m_grabber_item;
}

void NodeEditorScene::remove_node_item(const NodeId& node_id)
{
    auto it = m_nodes.find(node_id);
    if (it == m_nodes.end())
        return;

    m_updating_selection = true;
    m_move_items_cache.erase(it->second);
    removeItem(it->second);
    m_updating_selection = false;
    it->second->deleteLater();
    m_nodes.erase(node_id);
}

void NodeEditorScene::remove_connection_item(const ConnectionId& connection_id)
{
    auto it = m_connections.find(connection_id);
    if (it == m_connections.end())
        return;
    m_updating_selection = true;
    removeItem(it->second);
    if (auto start_node = get_item_for_node(m_graph_model->get_node_id_from_port(connection_id.start_port)))
        start_node->remove_connection(it->second);
    if (auto end_node = get_item_for_node(m_graph_model->get_node_id_from_port(connection_id.end_port)))
        end_node->remove_connection(it->second);
    m_updating_selection = false;
    delete it->second;
    m_connections.erase(it);
}

void NodeEditorScene::update_node(const NodeId& node_id)
{
    auto it = m_nodes.find(node_id);
    if (it != m_nodes.end())
        it->second->update_node();
}

void NodeEditorScene::update_color(const NodeId& node_id)
{
    auto it = m_nodes.find(node_id);
    if (it != m_nodes.end())
        it->second->update_color(node_id);
}

void NodeEditorScene::update_port(const PortId& port_id)
{
    const auto node_id = m_graph_model->get_node_id_from_port(port_id);
    auto it = m_nodes.find(node_id);
    if (it != m_nodes.end())
        it->second->update_port(port_id);
}

void NodeEditorScene::get_selection(QVector<NodeId>& nodes, QVector<ConnectionId>& connections) const
{
    nodes.clear();
    connections.clear();
    for (const auto item : selectedItems())
    {
        if (auto node_item = qgraphicsitem_cast<NodeItem*>(item))
            nodes.push_back(node_item->get_id());
        else if (auto connection_item = qgraphicsitem_cast<ConnectionItem*>(item))
            connections.push_back(connection_item->get_id());
    }
}

void NodeEditorScene::set_thumbnail_cache(ThumbnailCache* cache)
{
    m_thumbnail_cache = cache;
}

ThumbnailCache* NodeEditorScene::get_thumbnail_cache() const
{
    return m_thumbnail_cache;
}

void NodeEditorScene::begin_move(const QVector<NodeId>& nodes)
{
    for (const auto& node : nodes)
    {
        auto item = get_item_for_node(node);
        if (!item)
        {
            continue;
        }
        m_move_items_cache[item] = to_model_position(item->scenePos(), item->boundingRect().width());
    }
}

void NodeEditorScene::end_move()
{
    if (m_move_items_cache.empty())
        return;

    QVector<NodeId> ids(m_move_items_cache.size());
    QVector<QPointF> old_pos(m_move_items_cache.size());
    QVector<QPointF> new_pos(m_move_items_cache.size());

    int i = 0;
    for (auto it = m_move_items_cache.begin(); it != m_move_items_cache.end(); ++i, ++it)
    {
        ids[i] = it->first->get_id();
        old_pos[i] = it->second;
        new_pos[i] = to_model_position(it->first->scenePos(), it->first->boundingRect().width());
    }

    Q_EMIT nodes_moved(ids, old_pos, new_pos);
    m_move_items_cache.clear();
}

void NodeEditorScene::begin_resize(const NodeId& node)
{
    if (node.empty())
        return;

    auto item = get_item_for_node(node);
    if (!item)
    {
        return;
    }
    m_resize_cache = std::make_pair(item, item->boundingRect());
}

void NodeEditorScene::end_resize()
{
    if (!m_resize_cache.first || m_resize_cache.second == QRectF())
        return;

    Q_EMIT node_resized(m_resize_cache.first->get_id(), m_resize_cache.second.width(), m_resize_cache.second.height(),
                        m_resize_cache.first->boundingRect().width(), m_resize_cache.first->boundingRect().height());

    m_resize_cache.first = nullptr;
    m_resize_cache.second = QRectF();
}

QList<QGraphicsItem*> NodeEditorScene::get_items_from_snapping_rect(const QPointF& center_pos, const qreal& center_pos_offset,
                                                                    const qreal& side_length) const
{
    auto rect_pos = center_pos - QPointF(center_pos_offset, center_pos_offset);
    auto rect_size = QSize(side_length, side_length);
    auto cursor_snap_rect = QRectF(rect_pos, rect_size);
    return items(cursor_snap_rect);
}

OPENDCC_NAMESPACE_CLOSE
