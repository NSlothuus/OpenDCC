/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/material_editor/api.h"
#include "opendcc/ui/node_editor/node.h"
#include "opendcc/usd_editor/material_editor/model.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include "opendcc/usd_editor/material_editor/material_output_item.h"

class QGraphicsLinearLayout;
class QGraphicsWidget;

OPENDCC_NAMESPACE_OPEN

class OPENDCC_MATERIAL_EDITOR_API NodeGraphItem : public UsdPrimNodeItemBase
{
public:
    NodeGraphItem(MaterialGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_external);

    MaterialGraphModel& get_model();
    const MaterialGraphModel& get_model() const;

protected:
    virtual QVector<PropertyLayoutItem*> make_ports(const PXR_NS::UsdPrim& prim) override;
    virtual QString get_icon_path(const PXR_NS::UsdPrim& prim) const override;

    virtual PropertyLayoutItem* make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position) override;
};

class OPENDCC_MATERIAL_EDITOR_API NodeGraphOutputItem : public MaterialOutputItem
{
public:
    NodeGraphOutputItem(MaterialGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_input);

protected:
    virtual QString get_icon_path(const PXR_NS::UsdPrim& prim) const override;
    virtual QVector<PropertyLayoutItem*> make_ports(const PXR_NS::UsdPrim& prim) override;
    virtual PropertyLayoutItem* make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position) override;
};

OPENDCC_NAMESPACE_CLOSE
