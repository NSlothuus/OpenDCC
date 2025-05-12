// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/ui/node_editor/hydra_op_input_item.h"
#include "opendcc/hydra_op/ui/node_editor/hydra_op_graph_model.h"
#include "opendcc/ui/node_editor/connection.h"
#include <QGraphicsLayoutItem>
#include <QGraphicsLinearLayout>
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include "opendcc/hydra_op/schema/baseNode.h"
#include "opendcc/hydra_op/schema/group.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

HydraOpInputItem::HydraOpInputItem(HydraOpGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_input)
    : UsdPrimNodeItemBase(model, node_id, display_name, Orientation::Vertical, false, false)
    , m_is_input(is_input)
{
    if (is_input)
    {
        const auto port_id = node_id + "." + TfStringSplit(node_id, "#graph_in_")[1];
        m_output_port = new PropertyWithPortsLayoutItem(model, this, port_id, Port::Type::Output, false);
        m_output_port->setParentItem(this);
        m_output_port->setGeometry(QRectF(0, 0, s_port_height, s_port_height));
    }
}

bool HydraOpInputItem::is_input() const
{
    return m_is_input;
}

void HydraOpInputItem::invalidate_layout()
{
    UsdPrimNodeItemBase::invalidate_layout();
    const auto inputs_y = s_port_height;
    const auto center_x = get_prop_layout()->geometry().width() / 2;
    const auto icon_half_w = get_icon_item()->boundingRect().width() * get_icon_item()->scale() / 2;
    const auto icon_height = get_icon_item()->boundingRect().height() * get_icon_item()->scale();
    const auto icon_center_y = get_icon_item()->boundingRect().height() * get_icon_item()->scale() / 2;
    get_icon_item()->setPos(center_x - icon_half_w, inputs_y);

    const auto bbox = boundingRect();
    if (is_input())
    {
        m_output_port->setPos(QPointF(center_x - s_port_radius, get_icon_item()->pos().y() + icon_height + 5));
    }
    get_display_name_item()->setPos(bbox.width() + 15, get_display_name_item()->y());
}

QPointF HydraOpInputItem::get_node_pos() const
{
    auto model_pos = get_model().get_node_position(get_id());
    if (!model_pos.isNull())
    {
        return to_scene_position(model_pos, boundingRect().width());
    }

    QRectF scene_rect;
    for (const auto& item : scene()->items())
    {
        auto cur = item;
        while (cur)
        {
            if (!dynamic_cast<HydraOpInputItem*>(cur) && qgraphicsitem_cast<NodeItem*>(cur))
            {
                scene_rect |= cur->sceneBoundingRect();
                break;
            }
            cur = cur->parentItem();
        }
    }

    const auto this_rect = boundingRect();
    QPointF scene_pos;
    const auto vert_offset = 80;
    const auto mid_point = scene_rect.width() / 2 + scene_rect.x();

    if (is_input())
    {
        auto cur_input_name = get_model().get_property_name(m_output_port->get_id());
        const auto input_names = get_model().get_input_names(get_id());
        const auto inputs_offset = 150;
        const auto total_input_width = input_names.size() * (this_rect.width() + inputs_offset) - inputs_offset;

        qreal x_pos = mid_point - total_input_width / 2;
        for (int i = 0; i < input_names.size(); ++i)
        {
            if (cur_input_name == input_names[i])
            {
                x_pos += (this_rect.width() + inputs_offset) * i;
                break;
            }
        }

        return QPointF(x_pos, scene_rect.y() - this_rect.height() - vert_offset);
    }
    else
    {
        return QPointF(mid_point - this_rect.width() / 2, scene_rect.height() + scene_rect.y() + vert_offset);
    }
}

PropertyLayoutItem* HydraOpInputItem::make_port(const PortId& port_id, const UsdPrim& prim, int& position)
{
    if (is_input())
        return nullptr;

    if (UsdHydraOpGroup(prim))
    {
        auto result = new PropertyWithPortsLayoutItem(get_model(), this, port_id, Port::Type::Input, false);
        for (auto connection : get_prop_connections())
        {
            if (get_model().get_node_id_from_port(connection->get_id().end_port) == get_id())
            {
                result->add_connection(connection);
            }
        }
        return result;
    }

    return nullptr;
}

QString HydraOpInputItem::get_icon_path(const UsdPrim& prim) const
{
    return ":/icons/node_editor/withouttype";
}

QVector<PropertyLayoutItem*> HydraOpInputItem::make_ports(const UsdPrim& prim)
{
    if (is_input())
    {
        return {};
    }

    if (UsdHydraOpGroup(prim))
    {
        auto result = new PropertyWithPortsLayoutItem(get_model(), this, get_id() + ".outputs:out", Port::Type::Input, false);
        for (auto connection : get_prop_connections())
        {
            if (get_model().get_node_id_from_port(connection->get_id().end_port) == get_id())
            {
                result->add_connection(connection);
            }
        }
        return { result };
    }

    // for now
    return {};
}

const HydraOpGraphModel& HydraOpInputItem::get_model() const
{
    return static_cast<const HydraOpGraphModel&>(UsdPrimNodeItemBase::get_model());
}

HydraOpGraphModel& HydraOpInputItem::get_model()
{
    return static_cast<HydraOpGraphModel&>(UsdPrimNodeItemBase::get_model());
}

Port HydraOpInputItem::get_output_port_at(const QPointF& scene_pos) const
{
    QPainterPath port_shape;
    port_shape.addEllipse(get_header_out_port_center(), s_port_radius, s_port_radius);

    Port result;
    if (mapToScene(port_shape).contains(scene_pos))
    {
        result.id = get_id();
        result.type = Port::Type::Output;
        return result;
    }
    else
    {
        result.type = Port::Type::Unknown;
    }
    return result;
}

QPointF HydraOpInputItem::get_port_connection_pos(const Port& port) const
{
    if (is_input() && port.id == m_output_port->get_id())
    {
        const auto connection_pos = get_header_out_port_center();
        return mapToScene(connection_pos);
    }
    return UsdPrimNodeItemBase::get_port_connection_pos(port);
}

void HydraOpInputItem::move_connections()
{
    for (auto connection : m_prim_connections)
        move_connection_to_header(connection);
    UsdPrimNodeItemBase::move_connections();
}

void HydraOpInputItem::move_connection_to_header(ConnectionItem* item)
{
    // always outcoming
    auto con = static_cast<BasicConnectionItem*>(item);
    if (is_input())
    {
        con->set_start_pos(m_output_port->get_out_connection_pos());
    }
}

void HydraOpInputItem::add_connection(ConnectionItem* connection)
{
    if (!connection)
        return;
    if (is_input())
    {
        if (get_model().get_node_id_from_port(connection->get_id().start_port) == get_id())
        {
            m_prim_connections.insert(connection);
            move_connection_to_header(connection);
        }
    }
    else
    {
        UsdPrimNodeItemBase::add_connection(connection);
    }
}

void HydraOpInputItem::remove_connection(ConnectionItem* connection)
{
    m_prim_connections.remove(connection);
    UsdPrimNodeItemBase::remove_connection(connection);
}

OPENDCC_NAMESPACE_CLOSE
