/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include <memory>
#include <QGraphicsItem>
#include <QPen>

OPENDCC_NAMESPACE_OPEN

class NodeEditorView;
class NodeItem;

class NodeSnapper
{
public:
    NodeSnapper() = default;
    virtual QPointF try_snap(const QGraphicsItem& node_item) = 0;
    virtual ~NodeSnapper() {}
};

class OPENDCC_UI_NODE_EDITOR_API AlignSnapper : public NodeSnapper
{
public:
    AlignSnapper(const NodeEditorView& view);

    QPointF try_snap(const QGraphicsItem& node) override;
    void clear_snap_lines();
    ~AlignSnapper();

private:
    enum class SnappingSubject : char
    {
        CenterHorizontal,
        CenterVertical,
        Top,
        Bottom
    };

    enum SimSnapLines : short int
    {
        Horizontal,
        CenterVertical,
        SimSnapLinesCount
    };

    QPointF compute_candidate(const QRectF& snapping_rect, SnappingSubject snapping_subject, const QPointF& aligning_point,
                              const QGraphicsItem* node_ptr, QPointF& candidate_final);

    void update_snap_lines(const QPointF& start_point, const QPointF& end_point, const QSizeF& node_size, bool is_vertical);
    const NodeEditorView& m_view;

    QGraphicsLineItem* m_snap_lines[SimSnapLinesCount] = { nullptr, nullptr };
    QPen m_snap_pen;
    const qreal m_min_dist = 12.0f;
};

OPENDCC_NAMESPACE_CLOSE
