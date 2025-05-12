// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/common_widgets/round_marking_menu.h"

#include <qmath.h>
#include <QPoint>
#include <QRect>

OPENDCC_NAMESPACE_OPEN

RoundMarkingMenu::RoundMarkingMenu(const QPoint& global_pos, QWidget* parent /*= nullptr*/)
    : MarkingMenu(global_pos, parent)
{
}

QPoint RoundMarkingMenu::get_widget_pos(uint32_t action_index, const QRect& rect)
{
    auto offset = QPoint();
    const uint32_t n = m_menu_stack.top()->actions().size();
    const uint32_t next_pow2_number = qPow(2, std::ceil(std::log2(n))); //  1 << static_cast<uint32_t>(std::ceil(std::log2(n)));
    const double step = 360.0 / (next_pow2_number);

    const int tail = next_pow2_number - n;
    double degree = 0;
    if (action_index < n - tail)
        degree = step * action_index;
    else
        degree = (action_index - n + (next_pow2_number / 2)) * 2 * step;

    const auto c = qCos(qDegreesToRadians(degree) + M_PI / 2);
    const auto s = qSin(qDegreesToRadians(degree) + M_PI / 2);
    auto offset_x = -m_radius * c;
    auto offset_y = -m_radius * s;

    if (qFuzzyIsNull(c))
    {
        offset = QPoint(-rect.width() / 2, -rect.height() / 2);
    }
    else if (qFuzzyIsNull(s))
    {
        offset = QPoint(c < 0 ? 0 : -rect.width(), -rect.height() / 2);
    }
    else if (c > 0 && s > 0)
    {
        offset = QPoint(-rect.width(), 0);
    }
    else if (c > 0 && s < 0)
    {
        offset = QPoint(-rect.width(), -rect.height());
    }
    else if (c < 0 && s < 0)
    {
        offset = QPoint(0, -rect.height());
    }
    return offset + QPoint(offset_x, offset_y);
}
OPENDCC_NAMESPACE_CLOSE