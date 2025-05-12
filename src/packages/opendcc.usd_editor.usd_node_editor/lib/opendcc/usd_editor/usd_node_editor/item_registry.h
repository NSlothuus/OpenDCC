/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/usd_editor/usd_node_editor/api.h"
#include "opendcc/ui/node_editor/item_registry.h"

OPENDCC_NAMESPACE_OPEN

class UsdEditorGraphModel;
class BasicLiveConnectionItem;
class NodeEditorView;

class OPENDCC_USD_NODE_EDITOR_API UsdItemRegistry : public NodeEditorItemRegistry
{
public:
    UsdItemRegistry(UsdEditorGraphModel& model);
    ~UsdItemRegistry() = default;

    BasicLiveConnectionItem* make_live_connection(const NodeEditorScene& scene, const NodeEditorView& view, const Port& port);
    QGraphicsItem* make_live_node(const NodeEditorScene& scene, const std::string& type);
    virtual ConnectionItem* make_connection(const NodeEditorScene& scene, const ConnectionId& connection_id) override;
    virtual NodeItem* make_node(const NodeEditorScene& scene, const NodeId& node_id) override;

private:
    UsdEditorGraphModel& m_model;
};
OPENDCC_NAMESPACE_CLOSE
