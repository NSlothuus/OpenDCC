// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/material_editor/utils.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include "opendcc/ui/node_editor/graph_model.h"
#include "opendcc/ui/node_editor/scene.h"
#include "opendcc/ui/node_editor/view.h"
#include <QGraphicsSceneMouseEvent>
#include "opendcc/usd_editor/material_editor/shader_node.h"

using namespace OPENDCC_NAMESPACE;
PXR_NAMESPACE_USING_DIRECTIVE

namespace
{
    static void recursively_walk_on_connects(const GraphModel& model, NodeItem* node, std::unordered_set<QGraphicsItem*>& isolated_items)
    {
        assert(node);
        if (isolated_items.find(node) != isolated_items.end())
            return;

        auto scene = node->get_scene();

        isolated_items.insert(node);
        for (auto connection : scene->get_connection_items_for_node(node->get_id()))
        {
            const auto start_prim = model.get_node_id_from_port(connection->get_id().start_port);
            const auto end_prim = model.get_node_id_from_port(connection->get_id().end_port);
            // if is input
            if (end_prim == node->get_id())
            {
                isolated_items.insert(connection);
                recursively_walk_on_connects(model, scene->get_item_for_node(start_prim), isolated_items);
            }
        }
    }
};

void try_connect(GraphModel* model, NodeEditorScene* scene, NodeEditorView* view, QGraphicsSceneMouseEvent* event)
{
    assert(model);
    assert(scene);
    assert(view);
    assert(event);

    auto live_connection = dynamic_cast<BasicLiveConnectionItem*>(scene->get_grabber_item());
    if (!live_connection)
        return;

    const auto& source_port = live_connection->get_source_port();
    for (const auto item : view->items(view->mapFromScene(event->scenePos())))
    {
        if (auto node = dynamic_cast<UsdPrimNodeItemBase*>(item))
            node->reset_hover();
        if (auto prop_item = dynamic_cast<PropertyWithPortsLayoutItem*>(item))
        {
            auto end_port = prop_item->get_port_at(live_connection->get_end_pos());
            if (auto node = static_cast<UsdPrimNodeItemBase*>(prop_item->get_node_item()))
                node->reset_hover();
            if (end_port.type == Port::Type::Unknown)
                break;
            // clicked and released on the same port
            if (event->type() == QGraphicsSceneMouseEvent::GraphicsSceneMouseRelease && end_port.id == source_port.id)
                return;

            const auto inv_port_type = (source_port.type == Port::Type::Input) ? Port::Type::Output : Port::Type::Input;
            if (source_port.type == end_port.type || source_port.type == Port::Type::Unknown)
                end_port.type = inv_port_type;

            model->connect_ports(source_port, end_port);
            break;
        }
        if (auto group_item = dynamic_cast<PropertyGroupItem*>(item))
        {
            auto end_port = group_item->select_port(Port::Type::Input);
            if (auto node = group_item->get_node_item())
                node->reset_hover();
            if (end_port.type == Port::Type::Unknown)
                break;
            // clicked and released on the same port
            if (event->type() == QGraphicsSceneMouseEvent::GraphicsSceneMouseRelease && end_port.id == source_port.id)
                return;

            const auto inv_port_type = (source_port.type == Port::Type::Input) ? Port::Type::Output : Port::Type::Input;
            if (source_port.type == end_port.type || source_port.type == Port::Type::Unknown)
                end_port.type = inv_port_type;

            model->connect_ports(source_port, end_port);
            break;
        }
    }

    scene->remove_grabber_item();
}

NodeId change_preview_shader(GraphModel* model, NodeEditorScene* scene, const NodeId& cur_preview_shader, const SdfPath& new_shader_path)
{
    auto cur_shader = dynamic_cast<ShaderNodeItem*>(scene->get_item_for_node(cur_preview_shader));
    if (cur_shader)
        cur_shader->enable_preview(false);
    if (new_shader_path.IsEmpty())
    {
        for (auto item : scene->items())
        {
            if (qgraphicsitem_cast<NodeItem*>(item) || qgraphicsitem_cast<ConnectionItem*>(item))
                item->setOpacity(1.0);
        }
        return "";
    }

    auto shader_item = dynamic_cast<ShaderNodeItem*>(scene->get_item_for_node(new_shader_path.GetString()));
    if (!shader_item)
        return "";

    shader_item->enable_preview(true);
    std::unordered_set<QGraphicsItem*> affected_items;
    recursively_walk_on_connects(*model, shader_item, affected_items);

    for (auto item : scene->items())
    {
        if (qgraphicsitem_cast<NodeItem*>(item) || qgraphicsitem_cast<ConnectionItem*>(item))
        {
            if (affected_items.find(item) != affected_items.end())
                item->setOpacity(1);
            else
                item->setOpacity(0.4);
        }
    }
    return shader_item->get_id();
}
