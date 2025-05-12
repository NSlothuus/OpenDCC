/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include "opendcc/ui/node_editor/text_item.h"
#include "opendcc/usd_editor/usd_node_editor/node.h"
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>
#include <pxr/usd/usdUI/backdrop.h>
#include <QGraphicsItem>
#include <QTextEdit>

OPENDCC_NAMESPACE_OPEN

class BackdropSizerItem;
class NodeItem;
class NodeTextItem;
class NodeEditorView;
class UsdGraphModel;

class OPENDCC_USD_NODE_EDITOR_API BackdropLiveNodeItem : public QGraphicsRectItem
{
public:
    BackdropLiveNodeItem(UsdGraphModel& model, const PXR_NS::TfToken& name, const PXR_NS::TfToken& type, const PXR_NS::SdfPath& parent_path,
                         QGraphicsItem* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    NodeEditorScene* get_scene();

    UsdGraphModel& m_model;
    PXR_NS::SdfPath m_parent_path;
    PXR_NS::TfToken m_name;
    PXR_NS::TfToken m_type;
    QPointF m_start_pos;
};

class OPENDCC_USD_NODE_EDITOR_API BackdropNodeItem : public NodeItem
{
public:
    BackdropNodeItem(UsdGraphModel& model, const NodeId& node_id, const std::string& display_name);
    virtual ~BackdropNodeItem();
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void resize(float width, float height);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void on_sizer_pos_changed(QPointF pos);

    void add_connection(ConnectionItem* item) override;
    void remove_connection(ConnectionItem* item) override;
    UsdGraphModel& get_model();

protected:
    void update_node() override;
    void update_port(const PortId& port_id) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void move_connections() override;

private:
    void update_color(const PXR_NS::UsdUINodeGraphNodeAPI& prim);
    void update_size(const PXR_NS::UsdUINodeGraphNodeAPI& prim);
    void update_descr(const PXR_NS::UsdUIBackdrop& prim);
    void update_pos();
    void update_descr_font_scale();

    void update_title_vis();
    void update_descr_vis();

    void align_label();

    void update_overlapped_nodes_list();

    float m_width = 0;
    float m_height = 0;
    QPointF m_start_drag;
    bool m_dragging = false;
    BackdropSizerItem* m_sizer = nullptr;
    QVector<NodeId> m_nodes;
    QPointF m_start_pos;
    NodeTextItem* m_text_item = nullptr;
    NodeTextEditor* m_description_text_item = nullptr;
    QColor m_display_color;
    QString m_desc;
};

OPENDCC_NAMESPACE_CLOSE
