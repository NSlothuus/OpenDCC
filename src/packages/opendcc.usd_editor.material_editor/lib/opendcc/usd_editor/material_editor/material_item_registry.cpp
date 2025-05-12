// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/material_editor/material_item_registry.h"
#include "opendcc/usd_editor/material_editor/model.h"
#include "opendcc/ui/node_editor/scene.h"
#include "opendcc/ui/node_editor/connection.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include "opendcc/usd_editor/usd_node_editor/backdrop_node.h"
#include "opendcc/usd_editor/material_editor/shader_node.h"
#include "opendcc/app/core/command_utils.h"
#include "opendcc/usd_editor/material_editor/material_output_item.h"
#include "opendcc/usd_editor/material_editor/nodegraph_item.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include <pxr/usd/usdUI/backdrop.h>
#include <pxr/usd/usdShade/nodeGraph.h>
#include <pxr/usd/usdShade/material.h>

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN

MaterialEditorItemRegistry::MaterialEditorItemRegistry(MaterialGraphModel& model)
    : m_model(model)
{
}
MaterialEditorItemRegistry::~MaterialEditorItemRegistry() {}

QGraphicsItem* MaterialEditorItemRegistry::make_live_usd_node(const NodeEditorScene& scene, const PXR_NS::TfToken& type)
{
    const auto parent_prim = m_model.get_prim_for_node(m_model.get_root().GetString());
    const auto name = commands::utils::get_new_name_for_prim(TfToken(type), parent_prim);
    if (type == "Backdrop")
        return new BackdropLiveNodeItem(m_model, name, TfToken(type), parent_prim.GetPath());
    else if (type == "NodeGraph")
        return new UsdLiveNodeItem(m_model, name, TfToken(type), parent_prim.GetPath());

    return nullptr;
}

QGraphicsItem* MaterialEditorItemRegistry::make_live_shader_node(const NodeEditorScene& scene, const TfToken& shader_name, const TfToken& shader_id)
{
    const auto parent_prim = m_model.get_prim_for_node(m_model.get_root().GetString());
    const auto name = commands::utils::get_new_name_for_prim(shader_name, parent_prim);
    return new LiveShaderNodeItem(m_model, name, shader_id, parent_prim.GetPath());
}

BasicLiveConnectionItem* MaterialEditorItemRegistry::make_live_connection(const NodeEditorScene& scene, const NodeEditorView& view, const Port& port)
{
    const auto node_id = m_model.get_node_id_from_port(port.id);
    auto node_item = static_cast<UsdPrimNodeItemBase*>(scene.get_item_for_node(node_id));
    if (!node_item)
        return nullptr;

    const auto pos = node_item->get_port_connection_pos(port);
    return pos.isNull() ? nullptr : new BasicLiveConnectionItem(m_model, pos, port, new UsdConnectionSnapper(view, m_model), true);
}

ConnectionItem* MaterialEditorItemRegistry::make_connection(const NodeEditorScene& scene, const ConnectionId& connection_id)
{
    const auto start_node = m_model.get_node_id_from_port(connection_id.start_port);
    const auto end_node = m_model.get_node_id_from_port(connection_id.end_port);

    const auto start_node_item = static_cast<UsdPrimNodeItemBase*>(scene.get_item_for_node(start_node));
    if (!start_node_item)
        return nullptr;

    const auto end_node_item = static_cast<UsdPrimNodeItemBase*>(scene.get_item_for_node(end_node));
    if (!end_node_item)
        return nullptr;

    return new BasicConnectionItem(m_model, connection_id, true);
}

NodeItem* MaterialEditorItemRegistry::make_node(const NodeEditorScene& scene, const NodeId& node_id)
{
    auto prim = m_model.get_prim_for_node(node_id);
    if (!prim)
        return nullptr;

    const auto is_external = m_model.is_external_node(node_id);
    const auto name_str = prim.GetName().GetString();
    if (UsdUIBackdrop(prim))
        return new BackdropNodeItem(m_model, node_id, name_str);

    if (node_id == m_model.get_root().GetString() + "#mat_in")
    {
        return UsdShadeMaterial(prim) ? new MaterialOutputItem(m_model, node_id, name_str, true)
                                      : new NodeGraphOutputItem(m_model, node_id, name_str, true);
    }
    if (node_id == m_model.get_root().GetString() + "#mat_out")
    {
        return UsdShadeMaterial(prim) ? new MaterialOutputItem(m_model, node_id, name_str, false)
                                      : new NodeGraphOutputItem(m_model, node_id, name_str, false);
    }
    if (UsdShadeNodeGraph(prim))
        return new NodeGraphItem(m_model, node_id, name_str, is_external);

    if (UsdShadeShader(prim))
        return new ShaderNodeItem(m_model, node_id, name_str, is_external);
    else
        return new UsdPrimNodeItem(m_model, node_id, name_str, is_external);
}

OPENDCC_NAMESPACE_CLOSE
