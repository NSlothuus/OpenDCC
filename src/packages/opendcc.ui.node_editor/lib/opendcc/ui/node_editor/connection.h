/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include "opendcc/ui/node_editor/graph_model.h"
#include "opendcc/ui/node_editor/scene.h"
#include <QPainterPath>
#include <QGraphicsPathItem>

OPENDCC_NAMESPACE_OPEN

class ConnectionItem : public QGraphicsObject
{
public:
    ConnectionItem(const GraphModel& model, const ConnectionId& connection_id)
        : m_model(model)
        , m_connection_id(connection_id)
    {
    }
    virtual ~ConnectionItem() {};

    enum
    {
        Type = static_cast<int>(NodeEditorScene::GraphicsItemType::Connection)
    };

    const GraphModel& get_model() const { return m_model; }
    NodeEditorScene* get_scene() const { return static_cast<NodeEditorScene*>(scene()); }
    const ConnectionId& get_id() const { return m_connection_id; }
    int type() const override { return Type; }

private:
    const GraphModel& m_model;
    ConnectionId m_connection_id;
};

class OPENDCC_UI_NODE_EDITOR_API BasicConnectionItem : public ConnectionItem
{
    Q_OBJECT
public:
    BasicConnectionItem(const GraphModel& model, const ConnectionId& connection_id, bool horizontal = true);
    ~BasicConnectionItem();
    virtual QRectF boundingRect() const override;
    virtual QPainterPath shape() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    void set_start_pos(const QPointF& start_pos);
    void set_end_pos(const QPointF& end_pos);

    QPointF get_start_pos() const { return m_start_pos; }
    QPointF get_end_pos() const { return m_end_pos; }

    bool is_horizontal() { return m_horizontal; }
Q_SIGNALS:
    void connection_is_hover(const ConnectionId* connection_id);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    QGraphicsPathItem* m_path_item = nullptr;
    QPainterPath m_shape;
    QRectF m_bbox;
    QPointF m_start_pos;
    QPointF m_end_pos;
    bool m_horizontal = true;
};

class ConnectionSnapper;
class OPENDCC_UI_NODE_EDITOR_API BasicLiveConnectionItem
    : public QObject
    , public QGraphicsPathItem
{
    Q_OBJECT
public:
    BasicLiveConnectionItem(GraphModel& model, const QPointF& start_pos, const Port& source_port, ConnectionSnapper* snapper = nullptr,
                            bool horizontal = true);
    virtual ~BasicLiveConnectionItem();

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    const Port& get_source_port() const;
    const QPointF& get_start_pos() const;
    const QPointF& get_end_pos() const;
Q_SIGNALS:
    void mouse_pressed(QGraphicsSceneMouseEvent* event);
    void mouse_released(QGraphicsSceneMouseEvent* event);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    NodeEditorScene* get_scene() const;

private:
    GraphModel& m_model;
    ConnectionSnapper* m_snapper = nullptr;
    QPointF m_start_pos;
    QPointF m_end_pos;
    Port m_source_port;
    bool m_horizontal = true;
    double m_dash_offset = 0;
};

class ConnectionSnapper : public QObject
{
public:
    ConnectionSnapper(QObject* parent = nullptr)
        : QObject(parent)
    {
    }
    virtual QPointF try_snap(const BasicLiveConnectionItem& live_connection) = 0;
};

class OPENDCC_UI_NODE_EDITOR_API PreConnectionSnapper
{
public:
    PreConnectionSnapper();
    virtual ~PreConnectionSnapper();

    void update_cover_connection(BasicConnectionItem* connection, const QPointF& cursor_pos);
    void update_cover_connection(BasicConnectionItem* connection, const QPointF& input_pos, const QPointF& output_pos);
    void clear_pre_connection_line();

private:
    bool is_equal_connections_pos(BasicConnectionItem* connection);

    QPointF m_connection_start_pos;
    QPointF m_connection_end_pos;

    QPen m_snap_pen;

    BasicConnectionItem* m_base_connection = nullptr;
    QGraphicsPathItem* m_start_pre_connection = nullptr;
    QGraphicsPathItem* m_end_pre_connection = nullptr;
};
OPENDCC_NAMESPACE_CLOSE
