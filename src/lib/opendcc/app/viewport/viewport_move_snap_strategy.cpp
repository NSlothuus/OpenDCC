// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_move_snap_strategy.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

ViewportRelativeSnapStrategy::ViewportRelativeSnapStrategy(double step)
{
    m_step = GfIsClose(step, 0, 0.000001) ? 1.0 : std::abs(step);
}

GfVec3d ViewportRelativeSnapStrategy::get_snap_point(const GfVec3d& start_pos, const GfVec3d& start_drag, const GfVec3d& cur_drag)
{
    GfVec3d delta = cur_drag - start_drag;
    for (int i = 0; i < delta.dimension; i++)
    {
        const auto div = delta[i] / m_step;
        const auto integer = static_cast<int64_t>(div);
        delta[i] = m_step * integer + m_step * std::round(div - integer);
    }
    return start_pos + delta;
}

ViewportAbsoluteSnapStrategy::ViewportAbsoluteSnapStrategy(double step)
{
    m_step = GfIsClose(step, 0, 0.000001) ? 1.0 : std::abs(step);
}

GfVec3d ViewportAbsoluteSnapStrategy::get_snap_point(const GfVec3d& start_pos, const GfVec3d& start_drag, const GfVec3d& cur_drag)
{
    GfVec3d result = start_pos + cur_drag - start_drag;
    for (int i = 0; i < result.dimension; i++)
    {
        const auto div = result[i] / m_step;
        const auto integer = static_cast<int64_t>(div);
        result[i] = m_step * integer + m_step * std::round(div - integer);
    }
    return result;
}

OPENDCC_NAMESPACE_CLOSE