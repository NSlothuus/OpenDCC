/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/material_editor/api.h"
#include "opendcc/ui/node_editor/scene.h"
#include "opendcc/ui/node_editor/graph_model.h"
#include "opendcc/ui/node_editor/view.h"
#include <pxr/usd/sdf/path.h>
#include <QGraphicsSceneMouseEvent>

class QGraphicsSceneMouseEvent;

namespace OPENDCC_NAMESPACE
{

    class GraphModel;
    class NodeEditorScene;
    class NodeEditorView;

};

OPENDCC_MATERIAL_EDITOR_API void try_connect(OPENDCC_NAMESPACE::GraphModel* model, OPENDCC_NAMESPACE::NodeEditorScene* scene,
                                             OPENDCC_NAMESPACE::NodeEditorView* view, QGraphicsSceneMouseEvent* event);
OPENDCC_MATERIAL_EDITOR_API OPENDCC_NAMESPACE::NodeId change_preview_shader(OPENDCC_NAMESPACE::GraphModel* model,
                                                                            OPENDCC_NAMESPACE::NodeEditorScene* scene,
                                                                            const OPENDCC_NAMESPACE::NodeId& cur_preview_shader,
                                                                            const PXR_NS::SdfPath& new_shader_path);
