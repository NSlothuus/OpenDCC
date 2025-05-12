/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/base/qt_python.h"
#include "opendcc/ui/node_editor/node.h"
#include "opendcc/usd_editor/usd_node_editor/graph_model.h"
#include "opendcc/ui/node_editor/connection.h"
#include <pxr/usd/usd/prim.h>
#include <QGraphicsLayoutItem>
#include <QGraphicsLinearLayout>
#include <QGraphicsWidget>
#include <QTimer>
#include <memory>

class QGraphicsMouseEvent;
class QGraphicsWidget;
class QGraphicsSvgItem;
class QGraphicsLinearLayout;
OPENDCC_NAMESPACE_OPEN
class UsdGraphModel;
class UsdPrimNodeItem;
class BasicPortGraphicsItem;
class NodeTextItem;
class NodeEditorScene;
class HamGraphicsItem;
class UsdPrimNodeItemBase;
class PreConnectionSnapper;

// TODO: It should be moved to QStyle for this node editor
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_port_spacing;
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_port_radius;
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_port_width;
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_port_height;
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_node_width;
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_node_height;
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_port_vert_offset;
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_snapping_dist;
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_snapping_dist_sq;
extern OPENDCC_USD_NODE_EDITOR_API const qreal s_selection_pen_width;

class OPENDCC_USD_NODE_EDITOR_API UsdLiveNodeItem : public QGraphicsRectItem
{
public:
    UsdLiveNodeItem(UsdGraphModel& model, const PXR_NS::TfToken& name, const PXR_NS::TfToken& type, const PXR_NS::SdfPath& parent_path,
                    bool horizontal = true, QGraphicsItem* parent = nullptr);
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

protected:
    virtual void on_prim_created(const PXR_NS::UsdPrim& prim) {};

private:
    NodeEditorScene* get_scene();
    UsdGraphModel& m_model;
    PXR_NS::TfToken m_name;
    PXR_NS::TfToken m_type;
    PXR_NS::SdfPath m_parent_path;
    std::unique_ptr<PreConnectionSnapper> m_pre_connection;
};

class OPENDCC_USD_NODE_EDITOR_API PropertyLayoutItem
    : public QGraphicsLayoutItem
    , public QGraphicsObject
{
public:
    PropertyLayoutItem(UsdGraphModel& model, UsdPrimNodeItemBase* node, const PortId& id);
    ~PropertyLayoutItem() override {}

    enum
    {
        Type = static_cast<int>(NodeEditorScene::GraphicsItemType::Port)
    };

    enum class Data
    {
        DisplayText = 0
    };

    int type() const override { return Type; }
    const PortId& get_id() const { return m_id; }
    virtual void add_connection(ConnectionItem* connection) {};
    virtual void remove_connection(ConnectionItem* connection) {};
    virtual bool has_connections() const { return false; };
    virtual void move_connections() {};
    const UsdGraphModel& get_model() const { return m_model; }
    UsdGraphModel& get_model() { return m_model; }
    NodeEditorScene* get_scene() const { return static_cast<NodeEditorScene*>(scene()); }
    UsdPrimNodeItemBase* get_node_item() const;
    virtual bool try_snap(const BasicLiveConnectionItem& connection, QPointF& snap_point) const { return false; };
    void set_group(const std::string& name) { m_group_name = name; };
    const std::string& get_group() const { return m_group_name; }

private:
    UsdGraphModel& m_model;
    UsdPrimNodeItemBase* m_node = nullptr;
    PortId m_id;
    std::string m_group_name;
};

class NamedPropertyLayoutItem;

class OPENDCC_USD_NODE_EDITOR_API PropertyWithPortsLayoutItem : public PropertyLayoutItem
{
public:
    PropertyWithPortsLayoutItem(UsdGraphModel& model, UsdPrimNodeItemBase* node, const PortId& id, Port::Type port_type, bool horizontal = true);
    ~PropertyWithPortsLayoutItem() override;
    virtual void add_connection(ConnectionItem* connection) override;
    virtual void remove_connection(ConnectionItem* connection) override;
    virtual bool has_connections() const override;
    virtual void move_connections() override;
    QSet<ConnectionItem*>& get_connections();

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /* = nullptr */) override;
    void set_port_brush(const QBrush& brush);
    const QBrush& get_port_brush() const;
    void set_port_pen(const QPen& brush);
    const QPen& get_port_pen() const;
    virtual QRectF boundingRect() const override;

    virtual QPointF get_in_connection_pos() const;
    virtual QPointF get_out_connection_pos() const;
    void setGeometry(const QRectF& rect) override;
    virtual Port get_port_at(const QPointF& point) const;
    Port::Type get_port_type() const;
    virtual bool try_snap(const BasicLiveConnectionItem& connection, QPointF& snap_point) const override;

    void set_radius(qreal radius) { m_radius = radius; }
    qreal get_radius() const { return m_radius; }

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const override;
    static qreal get_port_spacing();

    void draw_port(QPainter* painter, const QStyleOptionGraphicsItem* option, const QPointF& pos) const;
    QPainterPath get_port_shape(const QPointF& pos) const;
    QPointF get_port_center(Port::Type type) const;

private:
    QTimer* m_port_tooltip_timer = nullptr;
    QPointF m_pos_for_port_tooltip;

    QBrush m_port_brush;
    QPen m_port_pen;
    QSet<ConnectionItem*> m_connections;
    QPointF m_scene_mouse_pos;
    qreal m_radius = s_port_radius;
    Port::Type m_port_type = Port::Type::Unknown;
    bool m_horizontal = true;
};

class OPENDCC_USD_NODE_EDITOR_API NamedPropertyLayoutItem : public PropertyWithPortsLayoutItem
{
public:
    NamedPropertyLayoutItem(UsdGraphModel& model, UsdPrimNodeItemBase* node, const PortId& id, const PXR_NS::TfToken& name, Port::Type port_type);

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget /* = nullptr */) override;

private:
    QString m_text;
    QFont m_text_font;
    QPen m_text_pen;
};

class OPENDCC_USD_NODE_EDITOR_API PropertyGroupItem : public QGraphicsWidget
{
    Q_OBJECT
public:
    PropertyGroupItem(UsdPrimNodeItemBase* node, const QString& name);
    virtual ~PropertyGroupItem() = default;

    enum
    {
        Type = static_cast<int>(NodeEditorScene::GraphicsItemType::Group)
    };
    int type() const override { return Type; }

    void add_item(PropertyLayoutItem* item);
    void set_name(const QString& name);
    void on_mouse_release(QGraphicsSceneMouseEvent* event);
    bool get_open_state() const;
    int get_prop_count() const;
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF& constraint = QSizeF()) const override;
    PropertyLayoutItem* get_prop_item(int index) const;
    void show_minimized();
    void set_visible(bool state);
    void set_expanded(bool state);
    void move_connections_to_header(PropertyLayoutItem*);
    virtual Port select_port(Port::Type type) const;
    QVector<PropertyLayoutItem*> get_more_ports(Port::Type type) const;
    virtual bool try_snap(const BasicLiveConnectionItem& connection, QPointF& snap_point) const;
    UsdPrimNodeItemBase* get_node_item() const { return m_node; }
    const QString& get_group_name() const;

Q_SIGNALS:
    void open_state_changed(bool state);

private:
    void update_ui_api(bool state);
    void setup_expansion_state();

    void update_layouts();

    class GroupHeaderWidget;

    QGraphicsLinearLayout* m_properties_layout;
    GroupHeaderWidget* m_head;
    UsdPrimNodeItemBase* m_node;

    bool m_content_visible = true;
    bool m_minimized = false;
};

class OPENDCC_USD_NODE_EDITOR_API MorePortLayoutItem : public NamedPropertyLayoutItem
{
public:
    MorePortLayoutItem(UsdGraphModel& model, UsdPrimNodeItemBase* node, const PortId& id);
    virtual Port get_port_at(const QPointF& point) const;
    virtual bool try_snap(const BasicLiveConnectionItem& connection, QPointF& snap_point) const override;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Port select_port(Port::Type type) const;
    void add_property_to_menu(PropertyWithPortsLayoutItem* item, QMenu* menu, Port::Type type) const;
};

class NodeItemGeometry;
class DisconnectFSM;
class DisconnectAfterShakeCommand;
class OPENDCC_USD_NODE_EDITOR_API UsdPrimNodeItemBase : public NodeItem
{
public:
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    UsdPrimNodeItemBase(UsdGraphModel& model, const NodeId& node_id, const std::string& display_name, Orientation orient = Orientation::Horizontal,
                        bool can_rename = true, bool show_full_path = false);
    ~UsdPrimNodeItemBase() override;

    virtual void set_expansion_state(const PXR_NS::TfToken& new_state);
    const PXR_NS::TfToken& get_expansion_state() const;
    void update_node() override;
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void add_connection(ConnectionItem* connection) override;
    virtual void remove_connection(ConnectionItem* connection) override;

    bool is_in_header_port_area(const QPointF& pos, Port::Type header_type) const;
    virtual QPointF get_port_connection_pos(const Port& port) const;
    QGraphicsLinearLayout* get_prop_layout() const;
    PropertyLayoutItem* get_layout_item_for_port(const PortId& port) const;
    virtual void invalidate_layout();
    UsdGraphModel& get_model();
    const UsdGraphModel& get_model() const;
    virtual void reset_hover();
    virtual void update_color(const NodeId& node_id) override;
    virtual void update_port(const PortId& port_id) override;
    virtual QVector<PropertyLayoutItem*> get_more_ports(Port::Type type) const;

    virtual bool try_snap(const BasicLiveConnectionItem& live_connection, QPointF& snap_pos) const;

    virtual void set_all_groups(bool is_expanded);

    struct InsertionData
    {
        bool can_cut = false;

        Port connection_start;
        Port connection_end;
        Port node_input;
        Port node_output;

        std::vector<ConnectionId> all_connection_with_node_input;
        std::vector<ConnectionId> all_connection_with_node_output;

        std::vector<Port> all_input_from_connection;
        std::vector<Port> all_output_from_connection;

        bool all_port_data_is_valid() const;
        bool all_vectors_fill() const;
        bool cut_data_is_empty() const;
    };
    bool can_insert_into_connection(const QPointF& pos, InsertionData& data);
    void insert_node_into_connection(const InsertionData& data);
    bool can_disconnect_after_shake(InsertionData& data);
    void cut_node_from_connection(const InsertionData& data);
    void set_disconnect_mode(bool is_shaked) { m_disconnected = is_shaked; }

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void move_connections() override;
    virtual void update_ports(const PXR_NS::UsdPrim& prim);
    virtual void on_update_expansion_state();
    void update_pos();
    void update_color();
    void update_icon(const PXR_NS::UsdPrim& prim);
    void update_expansion_state();
    virtual QVector<PropertyLayoutItem*> make_ports(const PXR_NS::UsdPrim& prim) = 0;
    virtual PropertyLayoutItem* make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position) = 0;

    virtual std::vector<PropertyWithPortsLayoutItem*> get_port_items() const;
    virtual std::vector<Port> get_ports() const;
    QSet<ConnectionItem*>& get_prop_connections();

    virtual void move_connection_to_header(ConnectionItem* item);
    virtual void move_connection_to_group(ConnectionItem* item, const PropertyGroupItem* group);
    virtual QString get_icon_path(const PXR_NS::UsdPrim& prim) const = 0;
    QPointF get_header_in_port_center() const;
    QPointF get_header_out_port_center() const;
    std::unordered_map<std::string, PropertyGroupItem*>& get_prop_groups();
    virtual QPointF get_node_pos() const;

    void draw_header_ports(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr);
    QGraphicsTextItem* get_full_path_text_item() const;
    QGraphicsTextItem* get_display_name_item() const;
    QGraphicsItem* get_icon_item() const;

protected:
    QRectF get_body_rect() const;

private:
    QColor get_display_color() { return m_display_color; }
    QColor get_border_color() { return m_border_color; }
    QColor get_selected_border_color() { return m_selected_border_color; }

    void draw_pre_connection(const QPointF& cursor_pos);
    void reconnect_ports_to_insert(const Port& node_input, const Port& node_output, const Port& connection_start, const Port& connection_end);

    bool find_hovered_connection_ports(Port& start_port, Port& end_port, const QPointF& pos);
    virtual bool find_available_ports(Port& input, Port& output, const Port& connection_start, const Port& connection_end);
    virtual bool find_ports_for_connection(Port& start_port, Port& end_port, BasicConnectionItem* connection);

    bool has_connection(ConnectionItem* item) const;
    bool need_cut_from_connector(InsertionData& data);
    bool can_cut_from_connection(InsertionData& data);

    PXR_NS::TfToken m_expansion_state;

private:
    std::unique_ptr<PreConnectionSnapper> m_pre_connection;
    std::unique_ptr<DisconnectFSM> m_disconnect_fsm;
    std::shared_ptr<DisconnectAfterShakeCommand> m_disconnect_cmd;
    bool m_disconnected = false;

    QSet<ConnectionItem*> m_prop_connections;
    QString m_icon_path;
    int m_height = 0;
    std::unique_ptr<NodeItemGeometry> m_aligner;
    bool m_moved = false;
    bool m_dragging = false;

    QColor m_display_color = QColor(64, 64, 64);

    std::unordered_map<std::string, PropertyGroupItem*> m_prop_groups;
    QColor m_border_color = QColor(32, 32, 32);
    const QColor m_selected_border_color = QColor(0, 173, 240);
};

class OPENDCC_USD_NODE_EDITOR_API UsdPrimNodeItem : public UsdPrimNodeItemBase
{
public:
    UsdPrimNodeItem(UsdGraphModel& model, const NodeId& node_id, const std::string& display_name, bool is_external);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    void add_connection(ConnectionItem* connection) override;
    void remove_connection(ConnectionItem* connection) override;
    QPointF get_port_connection_pos(const Port& port) const override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    QVector<PropertyLayoutItem*> make_ports(const PXR_NS::UsdPrim& prim) override;
    PropertyLayoutItem* make_port(const PortId& port_id, const PXR_NS::UsdPrim& prim, int& position) override;
    QString get_icon_path(const PXR_NS::UsdPrim& prim) const override;
    void move_connections() override;

private:
    QSet<ConnectionItem*> m_prim_connections;
};

class OPENDCC_USD_NODE_EDITOR_API UsdConnectionSnapper : public ConnectionSnapper
{
public:
    UsdConnectionSnapper(const NodeEditorView& view, const UsdGraphModel& model);
    QPointF try_snap(const BasicLiveConnectionItem& live_connection) override;

private:
    const NodeEditorView& m_view;
    const UsdGraphModel& m_model;
};

OPENDCC_NAMESPACE_CLOSE
