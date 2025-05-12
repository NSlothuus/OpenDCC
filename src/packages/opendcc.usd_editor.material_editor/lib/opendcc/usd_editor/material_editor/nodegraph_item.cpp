// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/material_editor/nodegraph_item.h"
#include "opendcc/usd_editor/material_editor/model.h"
#include "opendcc/usd_editor/material_editor/shader_node.h"
#include "usd_fallback_proxy/core/usd_prim_fallback_proxy.h"
#include <pxr/usd/usdUI/tokens.h>
#include <pxr/usd/usdShade/nodeGraph.h>
#include <QGraphicsLinearLayout>

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

NodeGraphItem::NodeGraphItem(MaterialGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_external)
    : UsdPrimNodeItemBase(model, node_id, display_name, Orientation::Horizontal, true, is_external)
{
}

QString NodeGraphItem::get_icon_path(const PXR_NS::UsdPrim& prim) const
{
    return ":/icons/node_editor/nodegraph";
}

PropertyLayoutItem* NodeGraphItem::make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position)
{
    UsdPrimFallbackProxy proxy(prim);
    auto prop = proxy.get_property_proxy(SdfPath(port_id).GetNameToken());
    if (!prop)
        return nullptr;

    std::vector<ConnectionItem*> connections;
    for (auto connection : get_prop_connections())
    {
        if (connection->get_id().start_port == port_id || connection->get_id().end_port == port_id)
            connections.push_back(connection);
    }

    const auto name = SdfPath(port_id).GetNameToken();
    bool is_output = TfStringStartsWith(name, "outputs:");
    const auto stripped_name = TfToken(name.data() + name.GetString().find(SdfPathTokens->namespaceDelimiter) + 1);
    PropertyWithPortsLayoutItem* item = nullptr;
    item = new NamedPropertyLayoutItem(get_model(), this, port_id, stripped_name, is_output ? Port::Type::Output : Port::Type::Input);

    auto layout = get_prop_layout();
    if (!is_output)
    {
        // insert before #add_in_port
        position = layout->count() - 1;
    }
    else
    {
        // #add_out_port is at 0 pos
        // #add_in_port is at 1 pos
        position = 1;
        // insert before #add_out_port
        for (int i = 0; i < layout->count(); ++i)
        {
            if (auto prop_item = static_cast<PropertyLayoutItem*>(layout->itemAt(i)))
            {
                if (TfStringStartsWith(get_model().get_property_name(prop_item->get_id()), "#add_out_port"))
                {
                    position = i;
                    break;
                }
            }
        }
    }

    for (auto con : connections)
        item->add_connection(con);

    auto color_it = s_port_color.find(prop->get_type_name().GetType());
    item->set_port_brush(color_it == s_port_color.end() ? s_fallback_port_color : color_it->second);
    return item;
}

QVector<PropertyLayoutItem*> NodeGraphItem::make_ports(const UsdPrim& prim)
{
    auto ng = UsdShadeNodeGraph(prim);
    if (!ng)
        return {};

    UsdPrimFallbackProxy proxy(prim);
    std::vector<UsdPropertyProxyPtr> inputs;
    std::vector<UsdPropertyProxyPtr> outputs;
    for (const auto prop : proxy.get_all_property_proxies())
    {
        if (TfStringStartsWith(prop->get_name_token(), "inputs:"))
            inputs.push_back(prop);
        else if (TfStringStartsWith(prop->get_name_token(), "outputs:"))
            outputs.push_back(prop);
    }

    QVector<PropertyLayoutItem*> result;
    result.reserve(inputs.size() + outputs.size());
    for (const auto col : { &outputs, &inputs })
    {
        for (auto prop : *col)
        {
            const auto name = prop->get_name_token();
            const auto prop_path = prim.GetPath().AppendProperty(name);
            std::vector<ConnectionItem*> connections;
            for (auto connection : get_prop_connections())
            {
                if (get_model().to_usd_path(connection->get_id().start_port) == prop_path ||
                    get_model().to_usd_path(connection->get_id().end_port) == prop_path)
                    connections.push_back(connection);
            }

            const auto stripped_name = TfToken(name.data() + name.GetString().find(SdfPathTokens->namespaceDelimiter) + 1);
            auto item = new NamedPropertyLayoutItem(get_model(), this, prop_path.GetString(), stripped_name,
                                                    col == &outputs ? Port::Type::Output : Port::Type::Input);
            for (auto connection : connections)
                item->add_connection(connection);

            auto color_it = s_port_color.find(prop->get_type_name().GetType());
            item->set_port_brush(color_it == s_port_color.end() ? s_fallback_port_color : color_it->second);
            result.push_back(item);
        }

        PropertyWithPortsLayoutItem* add_btn;
        if (col == &outputs)
            add_btn = new PropertyWithPortsLayoutItem(get_model(), this, get_id() + ".#add_out_port", Port::Type::Output);
        else
            add_btn = new PropertyWithPortsLayoutItem(get_model(), this, get_id() + ".#add_in_port", Port::Type::Input);

        add_btn->set_port_brush(Qt::green);
        result.push_back(add_btn);
    }
    return std::move(result);
}

const MaterialGraphModel& NodeGraphItem::get_model() const
{
    return static_cast<const MaterialGraphModel&>(UsdPrimNodeItemBase::get_model());
}

MaterialGraphModel& NodeGraphItem::get_model()
{
    return static_cast<MaterialGraphModel&>(UsdPrimNodeItemBase::get_model());
}

NodeGraphOutputItem::NodeGraphOutputItem(MaterialGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_input)
    : MaterialOutputItem(model, node_id, display_name, is_input)
{
}

QVector<PropertyLayoutItem*> NodeGraphOutputItem::make_ports(const PXR_NS::UsdPrim& prim)
{
    auto result = MaterialOutputItem::make_ports(prim);
    if (!is_input())
    {
        auto add_btn = new PropertyWithPortsLayoutItem(get_model(), this, get_id() + ".#add_out_port", Port::Type::Input);
        add_btn->set_port_brush(Qt::green);
        result.push_back(add_btn);
    }
    return std::move(result);
}

OPENDCC_NAMESPACE::PropertyLayoutItem* OPENDCC_NAMESPACE::NodeGraphOutputItem::make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim,
                                                                                         int& position)
{
    auto result = MaterialOutputItem::make_port(port_id, prim, position);
    if (!is_input())
        position = get_prop_layout()->count() - 1;
    return result;
}

QString OPENDCC_NAMESPACE::NodeGraphOutputItem::get_icon_path(const PXR_NS::UsdPrim& prim) const
{
    return ":/icons/node_editor/nodegraph";
}

OPENDCC_NAMESPACE_CLOSE
