// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/anim_engine/ui/graph_editor/spline_widget_commands.h"
#include "opendcc/anim_engine/curve/curve.h"
#include "opendcc/anim_engine/ui/graph_editor/spline_widget.h"
#include "opendcc/anim_engine/core/engine.h"
#include "opendcc/anim_engine/core/session.h"

OPENDCC_NAMESPACE_OPEN

void get_selection_info(const std::map<AnimEngine::CurveId, SplineWidget::CurveData>& from_widget_curve_map,
                        std::map<AnimEngine::CurveId, SelectionInfo>& to_selection_data_map)
{
    for (auto it = from_widget_curve_map.begin(); it != from_widget_curve_map.end(); ++it)
    {
        auto curve_id = it->first;
        const SplineWidget::CurveData& curve_data = it->second;
        auto curve = AnimEngineSession::instance().current_engine()->get_curve(it->first);

        if ((curve_data.selected_keys.size() > 0) || (curve_data.selected_tangents.size() > 0))
        {
            to_selection_data_map[curve_id] = { curve_data.selected_keys, curve_data.selected_tangents };
        }
    }
}

void set_selection_info(const std::map<AnimEngine::CurveId, SelectionInfo>& from_selection_data_map,
                        std::map<AnimEngine::CurveId, SplineWidget::CurveData>& to_widget_curve_map)
{
    for (auto it = to_widget_curve_map.begin(); it != to_widget_curve_map.end(); ++it)
    {
        SplineWidget::CurveData& curve = it->second;
        curve.selected_keys.clear();
        curve.selected_tangents.clear();
    }

    for (auto it = from_selection_data_map.begin(); it != from_selection_data_map.end(); ++it)
    {
        auto curve_id = it->first;
        const SelectionInfo& selection_data = it->second;
        if (to_widget_curve_map.count(curve_id) > 0)
        {
            to_widget_curve_map.at(curve_id).selected_keys = selection_data.selected_keys;
            to_widget_curve_map.at(curve_id).selected_tangents = selection_data.selected_tangents;
        }
    }
}

QColor color_for_component(uint32_t component_idx)
{
    switch (component_idx)
    {
    case 0:
        return QColor(229, 0, 0);
    case 1:
        return QColor(0, 229, 0);
    case 2:
        return QColor(74, 146, 255);
    default:
        return QColor(178, 191, 178);
    }
}
OPENDCC_NAMESPACE_CLOSE
