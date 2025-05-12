/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include "opendcc/ui/node_editor/node_snapper.h"
#include <QGraphicsView>
#include <QLabel>
#include <QWidget>

#include <set>
#include <unordered_map>
#include <functional>

class QMenu;

OPENDCC_NAMESPACE_OPEN

class TabSearchMenu;
class NodeItem;
class ConnectionItem;
class NodeEditorScene;
class BottomHintWidget;

class OPENDCC_UI_NODE_EDITOR_API NodeEditorView : public QGraphicsView
{
    Q_OBJECT
public:
    NodeEditorView(NodeEditorScene* scene, QWidget* parent = nullptr);
    virtual ~NodeEditorView();
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

    void fit_to_view();

    NodeEditorScene* get_node_scene() const;
    void set_tab_menu(TabSearchMenu* tab_menu);
    TabSearchMenu* get_tab_menu() const;
    BottomHintWidget* get_hint_widget();

    virtual bool focusNextPrevChild(bool next) override;
    NodeItem* node_at(const QPoint& pos) const;

    enum class GridType : char
    {
        NoGrid,
        GridPoints,
        GridLines
    };

    void set_grid_type(GridType grid_type);

    const std::unique_ptr<AlignSnapper>& get_align_snapper() const;
    std::unique_ptr<AlignSnapper>& get_align_snapper();
    void clear_align_lines();

    void enable_align_snapping(bool isEnabled);
    void update_scene();

Q_SIGNALS:
    // TODO: not sure if it needed because this data can be accessed from scene's signal
    void scene_rect_changed();
    void rect_changed();

protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
    virtual void paintEvent(QPaintEvent* event) override;

private:
    void draw_grid_lines(QPainter* painter, const QRectF& rect, QPen& pen, int grid_size) const;
    void draw_grid_points(QPainter* painter, const QRectF& rect, QPen& pen, int grid_size, int point_size) const;

    void update_rubber_band(QMouseEvent* event);

    void clear_rubber_band();
    QRegion get_rubber_band_region(const QWidget* widget, const QRect& rect) const;
    void update_rubber_band_region();

    TabSearchMenu* m_tab_menu = nullptr;
    QPointF m_last_mouse_pos;
    QPointF m_mouse_press_scene_pos;
    QPointF m_last_mouse_move_scene_pos;
    QPointF m_last_rubber_band_scene_pos;
    QPointF m_mouse_press_view_pos;
    QRectF m_scene_range;
    bool m_pan_mode = false;

    bool m_rubber_banding = false;
    QRect m_rubber_band_rect;
    Qt::ItemSelectionOperation m_selection_operation;

    BottomHintWidget* m_hint = nullptr;
    GridType m_grid_type = GridType::NoGrid;
    std::unique_ptr<AlignSnapper> m_align_snapper = nullptr;
};

class OPENDCC_UI_NODE_EDITOR_API BottomHintWidget : public QWidget
{
public:
    BottomHintWidget(NodeEditorView* view);
    void clear_text();
    void update_text(const QString& text);
    void update_rect();
    virtual void setVisible(bool visible) override;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    bool m_custom_visible = true;
    int m_koeff = 1;
    NodeEditorView* m_view = nullptr;
    QLabel* m_label = nullptr;
    QString m_currentText;
};

OPENDCC_NAMESPACE_CLOSE
