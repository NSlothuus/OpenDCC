/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/graph_model.h"

class QGraphicsItem;

OPENDCC_NAMESPACE_OPEN

class NodeEditorScene;
class ConnectionItem;
class NodeItem;

class NodeEditorItemRegistry
{
public:
    virtual ~NodeEditorItemRegistry() {};

    virtual ConnectionItem* make_connection(const NodeEditorScene& scene, const ConnectionId& connection_id) = 0;
    virtual NodeItem* make_node(const NodeEditorScene& scene, const NodeId& node_id) = 0;
};

OPENDCC_NAMESPACE_CLOSE
