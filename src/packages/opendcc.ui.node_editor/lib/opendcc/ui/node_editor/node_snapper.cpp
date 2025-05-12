// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/node_editor/node_snapper.h"
#include "opendcc/ui/node_editor/view.h"
#include "opendcc/ui/node_editor/node.h"

OPENDCC_NAMESPACE_OPEN

AlignSnapper::AlignSnapper(const NodeEditorView& view)
    : m_view(view)
{
    m_snap_pen.setColor(QColor(112, 136, 163));
    m_snap_pen.setWidth(1.8);
    m_snap_pen.setStyle(Qt::PenStyle::DashLine);
    m_snap_pen.setCapStyle(Qt::RoundCap);
}

QPointF AlignSnapper::try_snap(const QGraphicsItem& node)
{
    QPointF center = node.sceneBoundingRect().center();
    QPointF top_left = node.sceneBoundingRect().topLeft();
    QPointF bottom = QPointF(center.x(), node.sceneBoundingRect().bottomLeft().y());
    QPointF top = QPointF(center.x(), top_left.y());

    const QRectF& scene_range_rect = m_view.sceneRect();

    QRectF snap_rec_center_hor(scene_range_rect.left(), center.y() - m_min_dist, scene_range_rect.width(), 2 * m_min_dist);

    QRectF snap_rec_center_vert(center.x() - m_min_dist, scene_range_rect.top(), 2 * m_min_dist, scene_range_rect.height());

    QRectF snap_rec_bottom_hor(scene_range_rect.left(), bottom.y() - m_min_dist, scene_range_rect.width(), 2 * m_min_dist);

    QRectF snap_rec_top_hor(scene_range_rect.left(), top.y() - m_min_dist, scene_range_rect.width(), 2 * m_min_dist);

    struct
    {
        SnappingSubject snapping_subject;
        QPointF& snap_point;
        QRectF& snap_rect;
    } snapping_cases_info[] = {
        { SnappingSubject::CenterHorizontal, center, snap_rec_center_hor },
        { SnappingSubject::CenterVertical, center, snap_rec_center_vert },
        { SnappingSubject::Top, bottom, snap_rec_bottom_hor },
        { SnappingSubject::Bottom, bottom, snap_rec_bottom_hor },
        { SnappingSubject::Top, top, snap_rec_top_hor },
        { SnappingSubject::Bottom, top, snap_rec_top_hor },
    };

    QPointF node_end_pos_final;

    for (const auto& snap_case : snapping_cases_info)
    {
        auto snap_target_point = compute_candidate(snap_case.snap_rect, snap_case.snapping_subject, snap_case.snap_point, &node, node_end_pos_final);

        if (snap_target_point.isNull() && snap_case.snapping_subject == SnappingSubject::CenterVertical)
        {
            if (m_snap_lines[SimSnapLines::CenterVertical] != nullptr && m_snap_lines[SimSnapLines::CenterVertical]->scene() != nullptr)
            {
                m_view.get_node_scene()->removeItem(m_snap_lines[SimSnapLines::CenterVertical]);
            }
        }
        else if (node_end_pos_final.isNull())
        {
            if (m_snap_lines[SimSnapLines::Horizontal] != nullptr && m_snap_lines[SimSnapLines::Horizontal]->scene() != nullptr)
                m_view.get_node_scene()->removeItem(m_snap_lines[SimSnapLines::Horizontal]);
        }
        else if (!node_end_pos_final.isNull() && !snap_target_point.isNull())
        {
            update_snap_lines(snap_target_point, node_end_pos_final, node.sceneBoundingRect().size(),
                              snap_case.snapping_subject == SnappingSubject::CenterVertical);
        }
    }

    return node_end_pos_final;
}

void AlignSnapper::clear_snap_lines()
{
    for (auto snap_line : m_snap_lines)
        if (snap_line != nullptr && snap_line->scene() != nullptr)
            m_view.get_node_scene()->removeItem(snap_line);
}

AlignSnapper::~AlignSnapper()
{
    clear_snap_lines();
}

QPointF AlignSnapper::compute_candidate(const QRectF& snapping_rect, SnappingSubject snapping_subject, const QPointF& aligning_point,
                                        const QGraphicsItem* node_ptr, QPointF& candidate_final)
{
    bool is_vertical = (snapping_subject == SnappingSubject::CenterVertical);
    qreal dist_y = is_vertical ? std::numeric_limits<qreal>::max() : m_min_dist * m_min_dist;
    qreal dist_x = !is_vertical ? std::numeric_limits<qreal>::max() : m_min_dist * m_min_dist;

    QPointF target;
    QPointF node_pos;

    const auto selected_items = m_view.get_node_scene()->selectedItems();

    QPointF moving_point = node_ptr->sceneBoundingRect().topLeft();

    for (auto node : m_view.items(m_view.mapFromScene(snapping_rect)))
    {
        if ((!qgraphicsitem_cast<NodeItem*>(node)) ||
            (selected_items.contains(node) &&
             selected_items.contains(const_cast<QGraphicsItem*>(node_ptr)))) // QList::contains deduced type for pointers is T* const&
            continue;

        const auto& node_rect = node->sceneBoundingRect();
        const auto& node_rect_center = node_rect.center();

        QPointF snap_pos;
        switch (snapping_subject)
        {
        case SnappingSubject::CenterVertical:
        case SnappingSubject::CenterHorizontal:
        {
            snap_pos = node_rect_center;
            break;
        }
        case SnappingSubject::Bottom:
        {
            snap_pos = QPointF(node_rect_center.x(), node_rect.bottomLeft().y());
            break;
        }
        case SnappingSubject::Top:
        {
            snap_pos = QPointF(node_rect_center.x(), node_rect.topLeft().y());
            break;
        }
        };

        const auto perp_x = QPointF(aligning_point.x(), snap_pos.y());
        const auto dir_x = snap_pos - perp_x;
        qreal dist_x_sqr = QPointF::dotProduct(dir_x, dir_x);

        const auto perp_y = QPointF(snap_pos.x(), aligning_point.y());
        const auto dir_y = snap_pos - perp_y;
        const auto dist_y_sq = QPointF::dotProduct(dir_y, dir_y);

        if ((dist_y_sq <= dist_y) && (dist_x_sqr <= dist_x))
        {
            dist_y = dist_y_sq;
            dist_x = dist_x_sqr;
            target = snap_pos;
        }
    }

    if (!target.isNull())
    {
        const qreal dx = is_vertical ? (target.x() - aligning_point.x()) : 0.0f;
        const qreal dy = !is_vertical ? (target.y() - aligning_point.y()) : 0.0f;

        if (candidate_final.isNull())
        {
            candidate_final.setY(moving_point.y() + dy);
            candidate_final.setX(moving_point.x() + dx);
        }
        else
        {
            candidate_final.setY(is_vertical ? candidate_final.y() : moving_point.y() + dy);
            candidate_final.setX(is_vertical ? moving_point.x() + dx : candidate_final.x());
        }
    }

    return target;
}

void AlignSnapper::update_snap_lines(const QPointF& node_point_final, const QPointF& candidate_point_final, const QSizeF& node_size, bool is_vertical)
{
    auto node_scene = m_view.get_node_scene();
    if (is_vertical)
    {
        QLineF line;
        if (m_snap_lines[SimSnapLines::CenterVertical] != nullptr && m_snap_lines[SimSnapLines::CenterVertical]->scene() != nullptr)
            node_scene->removeItem(m_snap_lines[SimSnapLines::CenterVertical]);

        if (candidate_point_final.y() < node_point_final.y())
            line = QLineF(node_point_final, QPointF(node_point_final.x(), candidate_point_final.y() + node_size.height()));
        else
            line = QLineF(node_point_final, QPointF(node_point_final.x(), candidate_point_final.y()));

        m_snap_lines[SimSnapLines::CenterVertical] = node_scene->addLine(line);
        m_snap_lines[SimSnapLines::CenterVertical]->setZValue(4);
        m_snap_lines[SimSnapLines::CenterVertical]->setPen(m_snap_pen);
    }
    else
    {
        QLineF line;
        if (m_snap_lines[SimSnapLines::Horizontal] != nullptr && m_snap_lines[SimSnapLines::Horizontal]->scene() != nullptr)
            node_scene->removeItem(m_snap_lines[SimSnapLines::Horizontal]);

        line = QLineF(node_point_final, QPointF(candidate_point_final.x() + node_size.width() / 2.0f, node_point_final.y()));
        m_snap_lines[SimSnapLines::Horizontal] = node_scene->addLine(line);
        m_snap_lines[SimSnapLines::Horizontal]->setZValue(4);
        m_snap_lines[SimSnapLines::Horizontal]->setPen(m_snap_pen);

        if (m_snap_lines[SimSnapLines::CenterVertical] != nullptr && m_snap_lines[SimSnapLines::CenterVertical]->scene() != nullptr)
        {
            m_snap_lines[SimSnapLines::CenterVertical]->setLine(
                QLineF(m_snap_lines[SimSnapLines::CenterVertical]->line().p1(),
                       QPointF(m_snap_lines[SimSnapLines::CenterVertical]->line().p2().x(), candidate_point_final.y())));
        }
    }
}

OPENDCC_NAMESPACE_CLOSE
