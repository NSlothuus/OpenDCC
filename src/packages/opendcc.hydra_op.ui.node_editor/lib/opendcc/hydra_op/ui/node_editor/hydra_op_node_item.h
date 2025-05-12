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

class AddInputPort : public PropertyWithPortsLayoutItem
{
public:
    AddInputPort(UsdGraphModel& model, UsdPrimNodeItemBase* node, const PortId& id, Port::Type port_type, bool horizontal);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
};

class OPENDCC_HYDRA_OP_NODE_EDITOR_API HydraOpNodeItem : public UsdPrimNodeItemBase
{
public:
    HydraOpNodeItem(HydraOpGraphModel& model, const NodeId& node_id, const std::string& display_name, bool with_add_port, bool with_output = true);
    void add_connection(ConnectionItem* connection) override;
    void remove_connection(ConnectionItem* connection) override;
    QPointF get_port_connection_pos(const Port& port) const override;
    Port get_output_port_at(const QPointF& scene_pos) const;
    HydraOpGraphModel& get_model();
    const HydraOpGraphModel& get_model() const;

    void set_terminal_node(bool enable);
    virtual bool try_snap(const BasicLiveConnectionItem& live_connection, QPointF& snap_pos) const override;
    virtual void reset_hover() override;
    virtual void update_node() override;

protected:
    virtual QVector<PropertyLayoutItem*> make_ports(const PXR_NS::UsdPrim& prim) override;
    virtual std::vector<PropertyWithPortsLayoutItem*> get_port_items() const override;
    virtual QString get_icon_path(const PXR_NS::UsdPrim& prim) const override;

    virtual PropertyLayoutItem* make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position) override;
    void move_connections() override;
    void move_connection_to_header(ConnectionItem* item) override;
    void invalidate_layout() override;
    void update_port(const PortId& port_id) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    bool find_available_ports(Port& input, Port& output, const Port& connection_start, const Port& connection_end) override;
    bool find_ports_for_connection(Port& start_port, Port& end_port, BasicConnectionItem* connection) override;

private:
    Port make_output_port() const;
    void update_terminal_node_state(bool enable);

    bool m_is_bypass = false;
    QSet<ConnectionItem*> m_prim_connections;
    QGraphicsRectItem* m_terminal_btn = nullptr;
    PropertyWithPortsLayoutItem* m_output_port = nullptr;
    AddInputPort* m_add_input_item = nullptr;
    QGraphicsTextItem* m_type_name_item = nullptr;
    QGraphicsSvgItem* m_bypass_icon_item = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
