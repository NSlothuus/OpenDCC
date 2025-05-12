// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/ui/node_editor/hydra_op_node_item.h"
#include "opendcc/hydra_op/ui/node_editor/hydra_op_graph_model.h"
#include "opendcc/app/ui/node_icon_registry.h"
#include "opendcc/ui/node_editor/scene.h"
#include "opendcc/ui/node_editor/view.h"
#include "usd_fallback_proxy/core/usd_prim_fallback_proxy.h"
#include <pxr/usd/usdUI/sceneGraphPrimAPI.h>
#include <QGraphicsLinearLayout>
#include <QGraphicsSvgItem>
#include "opendcc/hydra_op/schema/baseNode.h"
#include <QGraphicsSceneEvent>
#include "QApplication"
#include <QPainter>
#include <QCursor>
#include <QPainterPath>
#include <QStyleOption>
#include <pxr/usd/usdShade/input.h>
#include "opendcc/hydra_op/schema/group.h"

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

namespace
{
    const QColor s_terminal_button_color = QColor(71, 71, 71);
    const QColor s_active_terminal_button_color = QColor(96, 96, 194);
    const QColor s_fallback_port_color = QColor(179, 179, 179);
    const QColor s_type_name_color = QColor(102, 102, 102);
    const qreal s_bypass_node_opacity = 0.4;

    bool is_input(const std::string& port_name)
    {
        return TfStringStartsWith(port_name, UsdHydraOpTokens->inputsIn);
    }
};

AddInputPort::AddInputPort(UsdGraphModel& model, UsdPrimNodeItemBase* node, const PortId& id, Port::Type port_type, bool horizontal)
    : PropertyWithPortsLayoutItem(model, node, id, port_type, horizontal)
{
}

void AddInputPort::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /*= nullptr*/)
{
    painter->save();
    const auto is_hovered = option->state.testFlag(QStyle::StateFlag::State_MouseOver);
    const auto is_enabled = option->state.testFlag(QStyle::StateFlag::State_Enabled);
    if (!is_enabled)
    {
        painter->setBrush(QColor(64, 64, 64).darker());
    }
    else if (is_hovered)
    {
        painter->setBrush(QColor(64, 64, 64).lighter());
    }
    else
    {
        painter->setBrush(QColor(64, 64, 64));
    }
    const auto rect = boundingRect();
    painter->setPen(Qt::NoPen);

    const auto radius = geometry().width() / 2;
    const auto diameter = geometry().width();
    painter->drawEllipse(rect.center(), radius, radius);

    const auto offset = 2.5;
    painter->fillRect(QRectF(rect.width() / 2 - 1, offset, 2, diameter - 2 * offset), !is_enabled  ? QColor(122, 122, 122).darker()
                                                                                      : is_hovered ? QColor(122, 122, 122).lighter()
                                                                                                   : QColor(122, 122, 122));
    painter->fillRect(QRectF(offset, rect.height() / 2 - 1, diameter - 2 * offset, 2), !is_enabled  ? QColor(122, 122, 122).darker()
                                                                                       : is_hovered ? QColor(122, 122, 122).lighter()
                                                                                                    : QColor(122, 122, 122));

    painter->restore();
}

void AddInputPort::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (get_port_shape(get_port_center(Port::Type::Input)).contains(event->scenePos()))
    {
        static_cast<HydraOpGraphModel&>(get_model()).add_input(get_node_item()->get_id(), "inputs:in");
        return;
    }
    PropertyWithPortsLayoutItem::mousePressEvent(event);
}

HydraOpNodeItem::HydraOpNodeItem(HydraOpGraphModel& model, const NodeId& node_id, const std::string& display_name, bool with_add_port,
                                 bool with_output)
    : UsdPrimNodeItemBase(model, node_id, display_name, Orientation::Vertical)
{
    QRect rect(0, 0, 15, 15);
    m_terminal_btn = new QGraphicsRectItem(rect, this);
    m_terminal_btn->setPen(Qt::NoPen);
    update_terminal_node_state(get_model().get_terminal_node() == node_id);

    const auto outputs = model.get_output_names(node_id);
    const auto output_port = make_output_port();

    m_output_port = new PropertyWithPortsLayoutItem(model, this, output_port.id, Port::Type::Output, false);
    m_output_port->setParentItem(this);
    m_output_port->setGeometry(QRectF(0, 0, s_port_height, s_port_height));
    if (!with_output)
        m_output_port->hide();

    m_type_name_item = new QGraphicsTextItem("", get_display_name_item());
    m_type_name_item->setDefaultTextColor(s_type_name_color);
    m_type_name_item->setTextInteractionFlags(Qt::NoTextInteraction);

    if (with_add_port)
    {
        m_add_input_item = new AddInputPort(model, this, node_id + ".#add_in_port", Port::Type::Input, false);
        m_add_input_item->setParentItem(this);
        m_add_input_item->set_radius(7.5);
        m_add_input_item->setGeometry(QRectF(0, 0, 15, 15));
    }

    m_is_bypass = get_model().is_node_bypassed(get_id());
    setOpacity(m_is_bypass ? s_bypass_node_opacity : 1.0);
}

void HydraOpNodeItem::remove_connection(ConnectionItem* connection)
{
    m_prim_connections.remove(connection);
    UsdPrimNodeItemBase::remove_connection(connection);
}

void HydraOpNodeItem::add_connection(ConnectionItem* connection)
{
    if (!connection)
        return;
    if (m_output_port && connection->get_id().start_port == m_output_port->get_id())
    {
        m_prim_connections.insert(connection);
        move_connection_to_header(connection);
    }
    else
    {
        UsdPrimNodeItemBase::add_connection(connection);
    }
}

QString HydraOpNodeItem::get_icon_path(const PXR_NS::UsdPrim& prim) const
{
    auto& registry = NodeIconRegistry::instance();
    if (registry.is_svg_exists(TfToken("USD"), prim.GetTypeName()))
    {
        return registry.get_svg(TfToken("USD"), prim.GetTypeName()).c_str();
    }
    else
    {
        return ":icons/node_editor/withouttype";
    }
}

PropertyLayoutItem* HydraOpNodeItem::make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position)
{
    std::vector<ConnectionItem*> connections;
    for (auto connection : get_prop_connections())
    {
        if (connection->get_id().end_port == port_id)
        {
            connections.push_back(connection);
        }
    }

    const auto name = SdfPath(port_id).GetNameToken();

    if (!is_input(name))
        return nullptr;
    auto item = new PropertyWithPortsLayoutItem(get_model(), this, port_id, Port::Type::Input, false);

    for (auto con : connections)
    {
        item->add_connection(con);
    }

    item->set_port_brush(s_fallback_port_color);
    return item;
}

void HydraOpNodeItem::move_connections()
{
    for (auto connection : m_prim_connections)
        move_connection_to_header(connection);
    UsdPrimNodeItemBase::move_connections();
}

void HydraOpNodeItem::move_connection_to_header(ConnectionItem* item)
{
    // always outcoming
    auto con = static_cast<BasicConnectionItem*>(item);
    con->set_start_pos(m_output_port->get_out_connection_pos());
}

void HydraOpNodeItem::invalidate_layout()
{
    UsdPrimNodeItemBase::invalidate_layout();
    const auto inputs_y = s_port_height;
    const auto center_x = get_prop_layout()->geometry().width() / 2;
    const auto icon_half_w = get_icon_item()->boundingRect().width() * get_icon_item()->scale() / 2;
    const auto icon_height = get_icon_item()->boundingRect().height() * get_icon_item()->scale();
    const auto icon_center_y = get_icon_item()->boundingRect().height() * get_icon_item()->scale() / 2;
    if (m_add_input_item)
    {
        m_add_input_item->setPos(center_x - m_add_input_item->boundingRect().width() / 2, inputs_y);
        get_icon_item()->setPos(center_x - icon_half_w, inputs_y + m_add_input_item->boundingRect().height() + 5);
    }
    else
    {
        get_icon_item()->setPos(center_x - icon_half_w, inputs_y);
    }
    const auto bbox = boundingRect();

    const auto rect = m_terminal_btn->rect();
    m_terminal_btn->setPos(bbox.width() - rect.width() - 15, get_icon_item()->y() + icon_center_y - rect.height() / 2);
    m_output_port->setPos(QPointF(center_x - s_port_radius, get_icon_item()->pos().y() + icon_height + 5));

    const auto type_name_rect = m_type_name_item->boundingRect();
    get_display_name_item()->setPos(bbox.width() + 15, bbox.height() / 2 + bbox.y());
    m_type_name_item->setPos(0, -0.8 * type_name_rect.height());
}

void HydraOpNodeItem::update_port(const PortId& port_id)
{
    UsdPrimNodeItemBase::update_port(port_id);

    const auto prop_name = TfToken(get_model().get_property_name(port_id));
    if (prop_name == UsdHydraOpTokens->inputsBypass || prop_name == UsdHydraOpTokens->hydraOpBypass)
    {
        m_is_bypass = get_model().is_node_bypassed(get_id());
        setOpacity(m_is_bypass ? s_bypass_node_opacity : 1.0);

        if (m_bypass_icon_item)
            m_bypass_icon_item->setVisible(m_is_bypass);

        if (m_is_bypass)
            update_node();
    }
}

void HydraOpNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        if (m_terminal_btn->contains(m_terminal_btn->mapFromScene(event->scenePos())))
        {
            set_terminal_node(true);
            return;
        }

        if (m_output_port->isVisible() && m_output_port->contains(m_output_port->mapFromScene(event->scenePos())))
        {
            Q_EMIT get_scene() -> port_pressed(make_output_port());
            return;
        }

        for (auto connection : m_prim_connections)
            connection->setZValue(4);
    }
    UsdPrimNodeItemBase::mousePressEvent(event);
}

void HydraOpNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_terminal_btn->contains(m_terminal_btn->mapFromScene(event->scenePos())))
    {
        return;
    }
    for (auto connection : m_prim_connections)
        connection->setZValue(2);
    UsdPrimNodeItemBase::mouseReleaseEvent(event);
}

void HydraOpNodeItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    auto grabber = get_scene()->get_grabber_item();
    auto live_connection = dynamic_cast<BasicLiveConnectionItem*>(grabber);
    if (!live_connection)
    {
        UsdPrimNodeItemBase::hoverEnterEvent(event);
        return;
    }

    const auto& source_port = live_connection->get_source_port();
    if (source_port.type == Port::Type::Input && m_add_input_item)
    {
        m_add_input_item->setEnabled(false);
    }
    m_output_port->setEnabled(get_model().can_connect(make_output_port(), live_connection->get_source_port()));
}

void HydraOpNodeItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    m_output_port->setEnabled(true);
    if (m_add_input_item)
        m_add_input_item->setEnabled(true);
    UsdPrimNodeItemBase::hoverLeaveEvent(event);
}

bool HydraOpNodeItem::find_available_ports(Port& input, Port& output, const Port& connection_start, const Port& connection_end)
{
    if (connection_start.id.empty() || connection_end.id.empty())
    {
        return false;
    }

    auto prim = get_model().get_prim_for_node(get_id());
    if (!prim) // || UsdHydraOpRender(prim))
    {
        return false;
    }

    if (get_prop_layout()->count() == 0 && get_model().has_add_port(get_model().get_prim_for_node(get_id())))
    {
        get_model().add_input(get_id(), "inputs:in");
    }

    for (int i = 0; i < get_prop_layout()->count(); ++i)
    {
        if (auto port_item = dynamic_cast<PropertyWithPortsLayoutItem*>(get_prop_layout()->itemAt(i)->graphicsItem()))
        {
            if (port_item->get_port_type() != Port::Type::Input)
            {
                continue;
            }

            Port port;
            port.type = Port::Type::Input;
            port.id = port_item->get_id();
            if (get_model().can_connect(connection_start, port) || get_model().can_connect(port, connection_end))
            {
                input = port;

                output = make_output_port();
                return true;
            }
        }
    }
    return false;
}

bool HydraOpNodeItem::find_ports_for_connection(Port& start_port, Port& end_port, BasicConnectionItem* connection)
{
    auto get_port_from_pos_lambda = [&](const QPointF& pos, const PortId& port_id) -> Port {
        Port port;
        port.id = port_id;

        for (auto item : get_scene()->get_items_from_snapping_rect(pos, s_port_radius + s_snapping_dist, s_port_width + 2 * s_snapping_dist))
        {
            if (auto port_item = dynamic_cast<PropertyWithPortsLayoutItem*>(item))
            {
                if (port_item->get_id() != port_id)
                {
                    continue;
                }
                port.type = port_item->get_port_type();
                return port;
            }
        }
        return Port();
    };

    auto start = get_port_from_pos_lambda(connection->get_start_pos(), connection->get_id().start_port);
    auto end = get_port_from_pos_lambda(connection->get_end_pos(), connection->get_id().end_port);

    if (start.id.empty() || end.id.empty() || start.type == end.type)
    {
        return false;
    }

    start_port = start;
    end_port = end;
    return true;
}

Port HydraOpNodeItem::make_output_port() const
{
    auto names = get_model().get_output_names(get_id());
    if (names.empty())
    {
        return {};
    }

    Port result;
    result.id = get_model().to_usd_path(get_id()).AppendProperty(TfToken(names[0])).GetString();
    result.type = Port::Type::Output;
    return result;
}

void HydraOpNodeItem::update_terminal_node_state(bool enable)
{
    if (enable)
    {
        m_terminal_btn->setBrush(s_active_terminal_button_color);
        m_terminal_btn->setData(QGraphicsItem::UserType, true);
    }
    else
    {
        m_terminal_btn->setBrush(s_terminal_button_color);
        m_terminal_btn->setData(QGraphicsItem::UserType, false);
    }
}

bool HydraOpNodeItem::try_snap(const BasicLiveConnectionItem& live_connection, QPointF& snap_pos) const
{
    qreal snap_dist = std::numeric_limits<qreal>::max();
    auto calc_dist_sq = [&live_connection, &snap_pos](qreal& cur_dist_sq, const QPointF& pos) {
        const auto start_to_center = pos - live_connection.get_end_pos();
        const auto dist_sq = QPointF::dotProduct(start_to_center, start_to_center);
        if (cur_dist_sq > dist_sq)
        {
            cur_dist_sq = dist_sq;
            snap_pos = pos;
        }
    };

    QPointF snap_candidate;
    if (m_add_input_item && m_add_input_item->try_snap(live_connection, snap_candidate))
    {
        calc_dist_sq(snap_dist, snap_candidate);
    }

    if (m_output_port->isVisible() && m_output_port->try_snap(live_connection, snap_candidate))
    {
        calc_dist_sq(snap_dist, snap_candidate);
    }

    if (UsdPrimNodeItemBase::try_snap(live_connection, snap_candidate))
    {
        calc_dist_sq(snap_dist, snap_candidate);
    }

    return snap_dist != std::numeric_limits<qreal>::max();
}

void HydraOpNodeItem::reset_hover()
{
    m_output_port->setEnabled(true);
    if (m_add_input_item)
        m_add_input_item->setEnabled(true);
}

void HydraOpNodeItem::set_terminal_node(bool enable)
{
    const auto enabled = m_terminal_btn->data(QGraphicsItem::UserType).toBool();
    if (enabled == enable)
        return;

    auto& model = get_model();
    // Check if the current id is a terminal node after set. There may be a situation where HydraOpSession reassignes
    // the terminal node to something else.
    update_terminal_node_state(enable && model.set_terminal_node(get_id()) && get_model().get_terminal_node() == get_id());
}

QVector<PropertyLayoutItem*> HydraOpNodeItem::make_ports(const UsdPrim& prim)
{
    std::vector<std::string> inputs;
    const auto node_inputs = get_model().get_input_names(get_id());
    for (const auto& input : node_inputs)
    {
        inputs.push_back(input);
    }

    QVector<PropertyLayoutItem*> result;
    result.reserve(inputs.size());
    for (const auto& prop : inputs)
    {
        const auto name = TfToken(prop);
        const auto prop_path = prim.GetPath().AppendProperty(name);
        std::vector<ConnectionItem*> connections;
        for (auto connection : get_prop_connections())
        {
            if (get_model().to_usd_path(connection->get_id().end_port) == prop_path)
            {
                connections.push_back(connection);
            }
        }

        const auto stripped_name = TfToken(name.data() + name.GetString().find(SdfPathTokens->namespaceDelimiter) + 1);
        auto item = new PropertyWithPortsLayoutItem(get_model(), this, prop_path.GetString(), Port::Type::Input, false);
        for (auto connection : connections)
        {
            item->add_connection(connection);
        }

        item->set_port_brush(s_fallback_port_color);
        result.push_back(item);
    }

    return result;
}

std::vector<PropertyWithPortsLayoutItem*> HydraOpNodeItem::get_port_items() const
{
    auto ports = UsdPrimNodeItemBase::get_port_items();

    if (true) //! UsdHydraOpRender(get_model().get_prim_for_node(get_id())))
    {
        if (m_output_port)
        {
            ports.push_back(m_output_port);
        }
    }

    return ports;
}

QPointF HydraOpNodeItem::get_port_connection_pos(const Port& port) const
{
    if (m_output_port && port.id == m_output_port->get_id())
    {
        const auto connection_pos = get_header_out_port_center();
        return mapToScene(connection_pos);
    }
    return UsdPrimNodeItemBase::get_port_connection_pos(port);
}

Port HydraOpNodeItem::get_output_port_at(const QPointF& scene_pos) const
{
    QPainterPath port_shape;
    port_shape.addEllipse(get_header_out_port_center(), s_port_radius, s_port_radius);

    if (mapToScene(port_shape).contains(scene_pos))
    {
        return make_output_port();
    }
    else
    {
        return Port();
    }
}

const HydraOpGraphModel& HydraOpNodeItem::get_model() const
{
    return static_cast<const HydraOpGraphModel&>(UsdPrimNodeItemBase::get_model());
}

HydraOpGraphModel& HydraOpNodeItem::get_model()
{
    return static_cast<HydraOpGraphModel&>(UsdPrimNodeItemBase::get_model());
}

void HydraOpNodeItem::update_node()
{
    auto prim = get_model().get_prim_for_node(get_id());
    assert(prim);
    auto type_name = prim.GetTypeName();
    m_type_name_item->setPlainText(type_name.GetText());

    UsdPrimNodeItemBase::update_node();

    if (get_icon_item())
    {
        if (!m_bypass_icon_item)
        {
            m_bypass_icon_item = new QGraphicsSvgItem(":/icons/node_editor/bypass", this);
            m_bypass_icon_item->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
        }

        auto bypass_rect = m_bypass_icon_item->boundingRect();
        auto scale_factor = 2 * boundingRect().height() / bypass_rect.height();
        m_bypass_icon_item->setScale(scale_factor);

        auto rect = boundingRect();
        auto new_x = rect.width() / 2 - bypass_rect.width() * scale_factor / 2;
        auto new_y = rect.height() / 2 - bypass_rect.height() * scale_factor / 2;
        m_bypass_icon_item->setPos(new_x, new_y);
        m_bypass_icon_item->setVisible(m_is_bypass);
    }
}

OPENDCC_NAMESPACE_CLOSE
