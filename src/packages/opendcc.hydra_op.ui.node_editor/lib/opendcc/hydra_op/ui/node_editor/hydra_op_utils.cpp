// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/ui/node_editor/hydra_op_utils.h"
#include "opendcc/hydra_op/ui/node_editor/hydra_op_node_item.h"
#include "opendcc/ui/node_editor/scene.h"
#include "opendcc/ui/node_editor/view.h"
#include <QGraphicsSceneEvent>

OPENDCC_NAMESPACE_USING

void try_connect(OPENDCC_NAMESPACE::GraphModel* model, OPENDCC_NAMESPACE::NodeEditorScene* scene, OPENDCC_NAMESPACE::NodeEditorView* view,
                 QGraphicsSceneMouseEvent* event)
{
    auto live_connection = dynamic_cast<BasicLiveConnectionItem*>(scene->get_grabber_item());
    if (!live_connection)
        return;

    const auto& source_port = live_connection->get_source_port();
    for (const auto item : view->items(view->mapFromScene(event->scenePos())))
    {
        if (auto node = dynamic_cast<HydraOpNodeItem*>(item))
        {
            node->reset_hover();
        }
        else if (auto prop_item = dynamic_cast<PropertyWithPortsLayoutItem*>(item))
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
    }

    scene->remove_grabber_item();
}

NodeId change_terminal_node(GraphModel* model, NodeEditorScene* scene, const NodeId& cur_terminal_node, const NodeId& new_terminal_node)
{
    auto old_node = dynamic_cast<HydraOpNodeItem*>(scene->get_item_for_node(cur_terminal_node));
    if (old_node)
        old_node->set_terminal_node(false);
    auto new_node = dynamic_cast<HydraOpNodeItem*>(scene->get_item_for_node(new_terminal_node));
    if (!new_node)
        return "";

    new_node->set_terminal_node(true);
    return new_node->get_id();
}
