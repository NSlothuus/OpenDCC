/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/material_editor/api.h"
#include "opendcc/usd_editor/material_editor/model.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include "opendcc/usd_editor/material_editor/utils.h"

class QGraphicsLinearLayout;
class QGraphicsWidget;

OPENDCC_NAMESPACE_OPEN

class OPENDCC_MATERIAL_EDITOR_API MaterialOutputItem : public UsdPrimNodeItemBase
{
public:
    MaterialOutputItem(MaterialGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_input);

    virtual void update_port(const PortId& port_id) override;
    void add_connection(ConnectionItem* connection) override;
    MaterialGraphModel& get_model();
    const MaterialGraphModel& get_model() const;

protected:
    virtual QVector<PropertyLayoutItem*> make_ports(const PXR_NS::UsdPrim& prim) override;
    virtual QString get_icon_path(const PXR_NS::UsdPrim& prim) const override;
    virtual PropertyLayoutItem* make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position) override;
    virtual QPointF get_node_pos() const override;
    bool is_input() const;

private:
    bool m_is_input = false;
};

OPENDCC_NAMESPACE_CLOSE
