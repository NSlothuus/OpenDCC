/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/hydra_op/ui/node_editor/api.h"
#include "opendcc/ui/node_editor/item_registry.h"
#include <pxr/base/tf/token.h>

OPENDCC_NAMESPACE_OPEN
class HydraOpGraphModel;
class BasicLiveConnectionItem;
class NodeEditorView;

class OPENDCC_HYDRA_OP_NODE_EDITOR_API HydraOpItemRegistry : public NodeEditorItemRegistry
{
public:
    HydraOpItemRegistry(HydraOpGraphModel& item_registry);
    ~HydraOpItemRegistry() override;

    QGraphicsItem* make_live_node(const NodeEditorScene& scene, const PXR_NS::TfToken& type);
    BasicLiveConnectionItem* make_live_connection(const NodeEditorScene& scene, const NodeEditorView& view, const Port& port);
    ConnectionItem* make_connection(const NodeEditorScene& scene, const ConnectionId& connection_id) override;
    NodeItem* make_node(const NodeEditorScene& scene, const NodeId& node_id) override;

private:
    HydraOpGraphModel& m_model;
};

OPENDCC_NAMESPACE_CLOSE
