/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include "opendcc/ui/node_editor/scene.h"
#include <unordered_set>

OPENDCC_NAMESPACE_OPEN

class ConnectionItem;
class GraphModel;

class OPENDCC_UI_NODE_EDITOR_API NodeItem : public QGraphicsObject
{
public:
    NodeItem(GraphModel& model, const NodeId& node_id);
    virtual ~NodeItem() {};

    enum
    {
        Type = static_cast<int>(NodeEditorScene::GraphicsItemType::Node)
    };
    virtual void update_node() {}
    virtual void update_color(const NodeId& node_id) {}
    virtual void update_port(const PortId& port_id) {}
    virtual void add_connection(ConnectionItem* connection) = 0;
    virtual void remove_connection(ConnectionItem* connection) = 0;

    int type() const override { return Type; }

    GraphModel& get_model() { return m_model; }
    const GraphModel& get_model() const { return m_model; }
    const NodeId& get_id() const { return m_id; }
    void set_id(const NodeId& id) { m_id = id; }
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    const NodeEditorScene* get_scene() const;
    NodeEditorScene* get_scene();

private:
    GraphModel& m_model;
    NodeId m_id;

protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void move_connections() = 0;
};

OPENDCC_NAMESPACE_CLOSE
