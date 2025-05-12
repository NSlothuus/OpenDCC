/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/hydra_op/ui/node_editor/api.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"

class QGraphicsLinearLayout;
class QGraphicsWidget;

OPENDCC_NAMESPACE_OPEN

class UsdGraphModel;
class HydraOpGraphModel;

class OPENDCC_HYDRA_OP_NODE_EDITOR_API HydraOpInputItem : public UsdPrimNodeItemBase
{
public:
    HydraOpInputItem(HydraOpGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_input);
    void add_connection(ConnectionItem* connection) override;
    void remove_connection(ConnectionItem* connection) override;

    QPointF get_port_connection_pos(const Port& port) const override;
    Port get_output_port_at(const QPointF& scene_pos) const;
    HydraOpGraphModel& get_model();
    const HydraOpGraphModel& get_model() const;
    void move_connection_to_header(ConnectionItem* item) override;
    void move_connections() override;

protected:
    virtual QVector<PropertyLayoutItem*> make_ports(const PXR_NS::UsdPrim& prim) override;
    virtual QString get_icon_path(const PXR_NS::UsdPrim& prim) const override;
    virtual PropertyLayoutItem* make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position) override;
    virtual QPointF get_node_pos() const override;

    void invalidate_layout() override;
    bool is_input() const;

private:
    PropertyWithPortsLayoutItem* m_output_port = nullptr;
    QSet<ConnectionItem*> m_prim_connections;
    bool m_is_input = true;
};

OPENDCC_NAMESPACE_CLOSE
