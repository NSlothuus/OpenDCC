// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/material_editor/material_output_item.h"
#include "opendcc/usd_editor/material_editor/model.h"
#include "opendcc/ui/node_editor/connection.h"
#include "opendcc/usd_editor/material_editor/model.h"
#include "opendcc/usd_editor/material_editor/shader_node.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include "usd_fallback_proxy/core/usd_prim_fallback_proxy.h"
#include <pxr/usd/usdUI/tokens.h>
#include <QGraphicsLinearLayout>

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN

namespace
{
    NodeId strip_suffix(const NodeId& id)
    {
        return TfStringGetBeforeSuffix(id, '#');
    }
};

MaterialOutputItem::MaterialOutputItem(MaterialGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_input)
    : UsdPrimNodeItemBase(model, node_id, display_name, Orientation::Horizontal, false)
    , m_is_input(is_input)
{
    QGraphicsTextItem* type = nullptr;
    if (is_input)
        type = new QGraphicsTextItem("Material Input", get_display_name_item());
    else
        type = new QGraphicsTextItem("Material Output", get_display_name_item());

    type->setDefaultTextColor(QColor(102, 102, 102));
    type->setTextInteractionFlags(Qt::NoTextInteraction);
    const auto text_rect = type->boundingRect();
    const auto x_off = type->mapFromItem(this, s_node_width / 2 - text_rect.width() / 2, 0);
    type->setPos(x_off.x(), -0.8 * text_rect.height());
}

void MaterialOutputItem::add_connection(ConnectionItem* connection)
{
    if (m_is_input)
    {
        if (get_model().get_node_id_from_port(connection->get_id().start_port) == get_id())
            UsdPrimNodeItemBase::add_connection(connection);
    }
    else
    {
        if (get_model().get_node_id_from_port(connection->get_id().end_port) == get_id())
            UsdPrimNodeItemBase::add_connection(connection);
    }
}

const MaterialGraphModel& MaterialOutputItem::get_model() const
{
    return static_cast<const MaterialGraphModel&>(UsdPrimNodeItemBase::get_model());
}

MaterialGraphModel& MaterialOutputItem::get_model()
{
    return static_cast<MaterialGraphModel&>(UsdPrimNodeItemBase::get_model());
}

QString MaterialOutputItem::get_icon_path(const PXR_NS::UsdPrim& prim) const
{
    return ":icons/node_editor/material";
}

QPointF MaterialOutputItem::get_node_pos() const
{
    auto model_pos = get_model().get_node_position(get_id());
    if (model_pos.isNull())
    {
        const auto rect = scene()->itemsBoundingRect();
        QPointF scene_pos;
        if (m_is_input)
            scene_pos = QPointF(rect.left() - 1.5 * boundingRect().width(), rect.center().y() + boundingRect().height() / 2);
        else
            scene_pos = QPointF(rect.right() + 0.5 * boundingRect().width(), rect.center().y() + boundingRect().height() / 2);
        return scene_pos;
    }
    return to_scene_position(model_pos, boundingRect().width());
}

bool MaterialOutputItem::is_input() const
{
    return m_is_input;
}

void MaterialOutputItem::update_port(const PortId& port_id)
{
    const auto prop_name = get_model().get_property_name(port_id);
    if (auto layout_item = dynamic_cast<PropertyWithPortsLayoutItem*>(get_layout_item_for_port(port_id)))
    {
        auto prim = get_model().get_prim_for_node(get_model().get_node_id_from_port(port_id));
        UsdPrimFallbackProxy proxy(prim);
        if (auto prop = proxy.get_property_proxy(TfToken(prop_name)))
        {
            auto color_it = s_port_color.find(prop->get_type_name().GetType());
            layout_item->set_port_brush(color_it == s_port_color.end() ? s_fallback_port_color : color_it->second);
        }
    }
    UsdPrimNodeItemBase::update_port(port_id);
}

PropertyLayoutItem* MaterialOutputItem::make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position)
{
    UsdPrimFallbackProxy proxy(prim);
    const auto prop_name = get_model().get_property_name(port_id);
    auto prop = proxy.get_property_proxy(TfToken(prop_name));
    if (!prop)
        return nullptr;

    if (m_is_input)
    {
        if (!TfStringStartsWith(prop_name, "inputs:"))
            return nullptr;
        position = get_prop_layout()->count() - 1;
    }
    else
    {
        if (!TfStringStartsWith(prop_name, "outputs:"))
            return nullptr;
    }

    const auto stripped_name = TfToken(prop_name.data() + prop_name.find(SdfPathTokens->namespaceDelimiter) + 1);
    auto result = new NamedPropertyLayoutItem(get_model(), this, port_id, stripped_name, m_is_input ? Port::Type::Output : Port::Type::Input);
    auto color_it = s_port_color.find(prop->get_type_name().GetType());
    result->set_port_brush(color_it == s_port_color.end() ? s_fallback_port_color : color_it->second);
    return result;
}

QVector<PropertyLayoutItem*> MaterialOutputItem::make_ports(const PXR_NS::UsdPrim& prim)
{
    UsdPrimFallbackProxy proxy(prim);
    QVector<PropertyLayoutItem*> result;
    for (const auto prop : proxy.get_all_property_proxies())
    {
        if (m_is_input && !TfStringStartsWith(prop->get_name_token(), "inputs:"))
            continue;
        if (!m_is_input && !TfStringStartsWith(prop->get_name_token(), "outputs:"))
            continue;

        const auto name = prop->get_name_token();
        const auto prop_path = prim.GetPath().AppendProperty(name);
        std::vector<ConnectionItem*> connections;
        for (auto connection : get_prop_connections())
        {
            if (get_model().to_usd_path(connection->get_id().start_port) == prop_path ||
                get_model().to_usd_path(connection->get_id().end_port) == prop_path)
            {
                connections.push_back(connection);
            }
        }

        const auto stripped_name = TfToken(name.data() + name.GetString().find(SdfPathTokens->namespaceDelimiter) + 1);
        PropertyWithPortsLayoutItem* item = nullptr;
        item = new NamedPropertyLayoutItem(get_model(), this, get_model().from_usd_path(prop_path, get_model().get_root()), stripped_name,
                                           m_is_input ? Port::Type::Output : Port::Type::Input);
        for (auto connection : connections)
            item->add_connection(connection);

        auto color_it = s_port_color.find(prop->get_type_name().GetType());
        item->set_port_brush(color_it == s_port_color.end() ? s_fallback_port_color : color_it->second);
        result.push_back(item);
    }

    if (m_is_input)
    {
        auto add_btn = new PropertyWithPortsLayoutItem(get_model(), this, get_id() + ".#add_in_port", Port::Type::Output);
        add_btn->set_port_brush(Qt::green);
        result.push_back(add_btn);
    }

    return std::move(result);
}

OPENDCC_NAMESPACE_CLOSE
