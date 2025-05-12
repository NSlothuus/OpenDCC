/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include <QGraphicsTextItem>
#include <QLineEdit>
#include <QTextEdit>
#include <QGraphicsProxyWidget>
#include <QEvent>

OPENDCC_NAMESPACE_OPEN

class NodeItem;

class OPENDCC_UI_NODE_EDITOR_API NodeTextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    using TryRenameFn = std::function<bool(const QString& new_name)>;

    NodeTextItem(const QString& text, const NodeItem& node_item, TryRenameFn can_rename, QGraphicsItem* parent = nullptr);
    const NodeItem& get_node_item() const { return m_node_item; }
    bool try_rename(const QString& new_name) { return m_try_rename(new_name); }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    const NodeItem& m_node_item;
    TryRenameFn m_try_rename;
};

class OPENDCC_UI_NODE_EDITOR_API EditNodeName : public QLineEdit
{
public:
    EditNodeName(NodeTextItem* text_item, size_t text_label_size, QWidget* parent = nullptr);

private:
    void editing_finished();
    void rename_with_validation();

    QString m_init_text;
    NodeTextItem* m_text_item = nullptr;
    bool m_renamed = false;
};

class OPENDCC_UI_NODE_EDITOR_API NodeTextEditor : public QGraphicsTextItem
{
    Q_OBJECT
public:
    NodeTextEditor(const QString& text, const QRectF& shape, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void update_bounding_rect(const QRectF& rect);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void set_font_scale(float scale);
    void set_sizer_size(float size);

private:
    int get_current_lines_count() const;
    void prepare_elide();

    QRectF m_bounding_rect;

    const short m_default_font_size = 5;
    float m_default_sizer_size = 0;
};

OPENDCC_NAMESPACE_CLOSE
