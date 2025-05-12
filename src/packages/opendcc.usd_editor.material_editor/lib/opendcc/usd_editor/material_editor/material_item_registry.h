/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/usd_editor/material_editor/api.h"
#include "opendcc/ui/node_editor/item_registry.h"
#include <pxr/base/tf/token.h>
OPENDCC_NAMESPACE_OPEN
class MaterialGraphModel;
class BasicLiveConnectionItem;
class NodeEditorView;

class OPENDCC_MATERIAL_EDITOR_API MaterialEditorItemRegistry : public NodeEditorItemRegistry
{
public:
    MaterialEditorItemRegistry(MaterialGraphModel& item_registry);
    ~MaterialEditorItemRegistry() override;

    QGraphicsItem* make_live_usd_node(const NodeEditorScene& scene, const PXR_NS::TfToken& type);
    QGraphicsItem* make_live_shader_node(const NodeEditorScene& scene, const PXR_NS::TfToken& shader_name, const PXR_NS::TfToken& shader_id);
    BasicLiveConnectionItem* make_live_connection(const NodeEditorScene& scene, const NodeEditorView& view, const Port& port);
    ConnectionItem* make_connection(const NodeEditorScene& scene, const ConnectionId& connection_id) override;
    NodeItem* make_node(const NodeEditorScene& scene, const NodeId& node_id) override;

private:
    MaterialGraphModel& m_model;
};

OPENDCC_NAMESPACE_CLOSE
