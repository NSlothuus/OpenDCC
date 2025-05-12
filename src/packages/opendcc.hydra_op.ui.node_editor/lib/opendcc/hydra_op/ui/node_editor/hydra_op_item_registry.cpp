// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/hydra_op/ui/node_editor/hydra_op_item_registry.h"
#include "opendcc/hydra_op/ui/node_editor/hydra_op_graph_model.h"
#include "opendcc/ui/node_editor/scene.h"
#include "opendcc/ui/node_editor/connection.h"
#include "opendcc/hydra_op/ui/node_editor/hydra_op_node_item.h"
#include "opendcc/usd_editor/usd_node_editor/backdrop_node.h"
#include "opendcc/hydra_op/schema/baseNode.h"
#include "opendcc/hydra_op/ui/node_editor/hydra_op_input_item.h"
#include "opendcc/hydra_op/schema/translateAPI.h"
#include "opendcc/hydra_op/schema/group.h"

PXR_NAMESPACE_USING_DIRECTIVE
OPENDCC_NAMESPACE_OPEN

HydraOpItemRegistry::HydraOpItemRegistry(HydraOpGraphModel& model)
    : m_model(model)
{
}
HydraOpItemRegistry::~HydraOpItemRegistry() {}

QGraphicsItem* HydraOpItemRegistry::make_live_node(const NodeEditorScene& scene, const PXR_NS::TfToken& type)
{
    const auto parent_prim = m_model.get_prim_for_node(m_model.get_root().GetString());

    if (m_model.is_supported_type_for_translate_api(type))
    {
        const auto name = commands::utils::get_new_name_for_prim(type, parent_prim);
        return new UsdLiveNodeItem(m_model, name, type, parent_prim.GetPath(), false);
    }

    std::set<TfType> hydra_op_types;
    TfType::Find<UsdHydraOpBaseNode>().GetAllDerivedTypes(&hydra_op_types);
    for (const auto& derived_type : hydra_op_types)
    {
        auto type_name = UsdSchemaRegistry::GetSchemaTypeName(derived_type);
        if (type_name == type)
        {
            constexpr auto hydra_op_name_offset = sizeof("HydraOp") - 1;
            const auto name = commands::utils::get_new_name_for_prim(TfToken(type.GetText() + hydra_op_name_offset), parent_prim);
            auto node_item = new UsdLiveNodeItem(m_model, name, type, parent_prim.GetPath(), false);

            return node_item;
        }
    }

    const auto name = commands::utils::get_new_name_for_prim(type, parent_prim);
    if (type == "Backdrop")
    {
        return new BackdropLiveNodeItem(m_model, name, TfToken(type), parent_prim.GetPath());
    }

    return new UsdLiveNodeItem(m_model, name, TfToken(type), parent_prim.GetPath(), false);
}

BasicLiveConnectionItem* HydraOpItemRegistry::make_live_connection(const NodeEditorScene& scene, const NodeEditorView& view, const Port& port)
{
    const auto node_id = m_model.get_node_id_from_port(port.id);
    auto node_item = static_cast<UsdPrimNodeItemBase*>(scene.get_item_for_node(node_id));
    if (!node_item)
        return nullptr;

    const auto pos = node_item->get_port_connection_pos(port);
    return pos.isNull() ? nullptr : new BasicLiveConnectionItem(m_model, pos, port, new UsdConnectionSnapper(view, m_model), false);
}

ConnectionItem* HydraOpItemRegistry::make_connection(const NodeEditorScene& scene, const ConnectionId& connection_id)
{
    const auto start_node = m_model.get_node_id_from_port(connection_id.start_port);
    const auto end_node = m_model.get_node_id_from_port(connection_id.end_port);

    const auto start_node_item = static_cast<UsdPrimNodeItemBase*>(scene.get_item_for_node(start_node));
    if (!start_node_item)
        return nullptr;

    const auto end_node_item = static_cast<UsdPrimNodeItemBase*>(scene.get_item_for_node(end_node));
    if (!end_node_item)
        return nullptr;

    return new BasicConnectionItem(m_model, connection_id, false);
}

NodeItem* HydraOpItemRegistry::make_node(const NodeEditorScene& scene, const NodeId& node_id)
{
    auto prim = m_model.get_prim_for_node(node_id);
    if (!prim)
        return nullptr;

    if (UsdHydraOpGroup(prim) && prim.GetPath() == m_model.get_root())
    {
        if (TfStringContains(node_id, "#graph_out"))
        {
            return new HydraOpInputItem(m_model, node_id, "HydraOpGroup Output", false);
        }
        else
        {
            return new HydraOpInputItem(m_model, node_id, TfStringSplit(node_id, "#graph_in_")[1], true);
        }
    }

    const auto name_str = prim.GetName().GetString();
    if (UsdUIBackdrop(prim))
    {
        return new BackdropNodeItem(m_model, node_id, name_str);
    }

    const auto has_output_port = true; //! UsdHydraOpRender(prim);

    if (UsdHydraOpBaseNode(prim) || UsdHydraOpTranslateAPI(prim))
    {
        return new HydraOpNodeItem(m_model, node_id, name_str, m_model.has_add_port(prim), has_output_port);
    }

    return nullptr;
}

OPENDCC_NAMESPACE_CLOSE
