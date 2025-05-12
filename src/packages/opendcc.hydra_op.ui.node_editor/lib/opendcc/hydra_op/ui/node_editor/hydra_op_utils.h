/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/hydra_op/ui/node_editor/api.h"
#include "opendcc/ui/node_editor/graph_model.h"

class QGraphicsSceneMouseEvent;
namespace OPENDCC_NAMESPACE
{
    class GraphModel;
    class NodeEditorScene;
    class NodeEditorView;
};

OPENDCC_HYDRA_OP_NODE_EDITOR_API void try_connect(OPENDCC_NAMESPACE::GraphModel* model, OPENDCC_NAMESPACE::NodeEditorScene* scene,
                                                  OPENDCC_NAMESPACE::NodeEditorView* view, QGraphicsSceneMouseEvent* event);
OPENDCC_HYDRA_OP_NODE_EDITOR_API OPENDCC_NAMESPACE::NodeId change_terminal_node(OPENDCC_NAMESPACE::GraphModel* model,
                                                                                OPENDCC_NAMESPACE::NodeEditorScene* scene,
                                                                                const OPENDCC_NAMESPACE::NodeId& cur_terminal_node,
                                                                                const OPENDCC_NAMESPACE::NodeId& new_terminal_node);
