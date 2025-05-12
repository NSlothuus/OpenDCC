// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/node_editor/node.h"
#include "opendcc/ui/node_editor/connection.h"
#include "opendcc/ui/node_editor/node_snapper.h"
#include "opendcc/ui/node_editor/view.h"
#include <QGraphicsSceneMouseEvent>

OPENDCC_NAMESPACE_OPEN
NodeItem::NodeItem(GraphModel& model, const NodeId& node_id)
    : m_model(model)
    , m_id(node_id)
{
    setAcceptHoverEvents(true);
}

const NodeEditorScene* NodeItem::get_scene() const
{
    return static_cast<const NodeEditorScene*>(scene());
}

NodeEditorScene* NodeItem::get_scene()
{
    return static_cast<NodeEditorScene*>(scene());
}

QVariant NodeItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == GraphicsItemChange::ItemScenePositionHasChanged)
        move_connections();
    return QGraphicsObject::itemChange(change, value);
}

void NodeItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
    if (auto scene = get_scene())
        Q_EMIT scene->node_double_clicked(get_id());
}

void NodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsItem::hoverEnterEvent(event);
    if (auto scene = get_scene())
        Q_EMIT scene->node_hovered(get_id());
}

void NodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsItem::hoverLeaveEvent(event);
    Q_EMIT get_scene() -> node_hovered(get_id(), false);
}

void NodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);

    if (const auto& snapper = get_scene()->get_view()->get_align_snapper())
    {
        auto snap = snapper->try_snap(*this);

        if (!snap.isNull())
        {
            const QPointF pos_delta = snap - pos();
            setPos(snap);

            for (auto selected_node : scene()->selectedItems())
            {
                if (selected_node != this && (selected_node->flags() & QGraphicsItem::GraphicsItemFlag::ItemIsMovable))
                    selected_node->setPos(selected_node->pos() + pos_delta);
            }
        }
    }
}

OPENDCC_NAMESPACE_CLOSE
