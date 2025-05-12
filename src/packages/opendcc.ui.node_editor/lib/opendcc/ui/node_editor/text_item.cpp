// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/node_editor/text_item.h"
#include "opendcc/ui/node_editor/node.h"
#include "opendcc/ui/node_editor/scene.h"
#include <QKeyEvent>
#include <QLineEdit>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QBoxLayout>
#include <QGraphicsView>
#include <QTextDocument>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QTimer>
#include <QTextBlock>

OPENDCC_NAMESPACE_OPEN

EditNodeName::EditNodeName(NodeTextItem* text_item, size_t text_label_size, QWidget* parent /*= nullptr*/)
    : QLineEdit(parent)
    , m_init_text(text_item->toPlainText())
    , m_text_item(text_item)
{
    assert(text_item);
    grabKeyboard();
    setText(text_item->toPlainText());
    setMaximumWidth(text_label_size * 1.5);
    connect(this, &QLineEdit::editingFinished, [this]() { editing_finished(); });
    connect(this, &QLineEdit::returnPressed, [this]() { rename_with_validation(); });
    setAttribute(Qt::WA_DeleteOnClose);
}

void EditNodeName::rename_with_validation()
{
    if (!m_text_item->try_rename(text()))
    {
    }
    else
    {
        m_renamed = true;
    }
}

void EditNodeName::editing_finished()
{
    m_text_item->show();
    if (!m_renamed)
        m_text_item->try_rename(text());
    releaseKeyboard();
    close();
}

NodeTextEditor::NodeTextEditor(const QString& text, const QRectF& shape, QGraphicsItem* parent)
    : QGraphicsTextItem(text, parent)
    , m_bounding_rect(shape)
{
    setTextInteractionFlags(Qt::NoTextInteraction);
    setTextWidth(shape.width());

    QFont text_font(font());
    text_font.setPointSize(m_default_font_size);
    setFont(text_font);

    qreal margin = 2.0f;
    auto doc = document();
    QTextFrameFormat format = doc->rootFrame()->frameFormat();
    doc->setDocumentMargin(margin);
    format.setBottomMargin(2 * margin);
    format.setRightMargin(margin);
    doc->rootFrame()->setFrameFormat(format);
}

QRectF NodeTextEditor::boundingRect() const
{
    return m_bounding_rect;
}

void NodeTextEditor::update_bounding_rect(const QRectF& rect)
{
    prepareGeometryChange();

    setTextWidth(rect.width());
    m_bounding_rect = rect;

    update();
}

void NodeTextEditor::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    prepare_elide();
    QGraphicsTextItem::paint(painter, option, widget);
}

void NodeTextEditor::set_font_scale(float scale)
{
    QFont new_font(font());
    new_font.setPointSizeF(m_default_font_size * scale);
    setFont(new_font);

    update();
}

void NodeTextEditor::set_sizer_size(float size)
{
    m_default_sizer_size = size;
}

int NodeTextEditor::get_current_lines_count() const
{
    const auto doc = document();
    int lines_count = 0;

    for (int i = 0; i < doc->blockCount(); i++)
    {
        auto text_block = doc->findBlockByNumber(i);
        auto layout = text_block.layout();
        if (layout == nullptr)
            return 0;

        lines_count += layout->lineCount();
    }

    return lines_count;
}

void NodeTextEditor::prepare_elide()
{
    auto line_count = get_current_lines_count();
    if (!line_count)
        return;

    QFontMetrics font_metrics(font());
    auto line_height = font_metrics.lineSpacing();
    int line_number = m_bounding_rect.height() / line_height;

    if (!line_number)
        return;

    if (line_count > line_number)
    {
        QTextCursor cursor(document());
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line_number - 1);
        cursor.select(QTextCursor::LineUnderCursor);

        QString elided_string = font_metrics.elidedText(cursor.selectedText(), Qt::ElideRight, textWidth() - m_default_sizer_size);
        cursor.insertText(elided_string);
        cursor.movePosition(QTextCursor::NextRow);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }
}

NodeTextItem::NodeTextItem(const QString& text, const NodeItem& node_item, TryRenameFn can_rename, QGraphicsItem* parent)
    : QGraphicsTextItem(text, parent)
    , m_node_item(node_item)
    , m_try_rename(can_rename)
{
    setTextInteractionFlags(Qt::NoTextInteraction);
}

void NodeTextItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (parentItem())
    {
        auto dialog = new EditNodeName(this, boundingRect().size().width(), nullptr);
        auto dialog_widget = scene()->addWidget(dialog);

        if (flags().testFlag(QGraphicsItem::ItemIgnoresTransformations))
            dialog_widget->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
        dialog->move(scenePos().x() - 3, scenePos().y() - 1);
        dialog->setFocus();

        hide();
    }
    QGraphicsTextItem::mousePressEvent(event);
}

OPENDCC_NAMESPACE_CLOSE
