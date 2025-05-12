/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QColor>
#include "spline_widget.h"

OPENDCC_NAMESPACE_OPEN

void get_selection_info(const std::map<OPENDCC_NAMESPACE::AnimEngine::CurveId, SplineWidget::CurveData>& from_widget_curve_map,
                        std::map<OPENDCC_NAMESPACE::AnimEngine::CurveId, SelectionInfo>& to_selection_data_map);
void set_selection_info(const std::map<OPENDCC_NAMESPACE::AnimEngine::CurveId, SelectionInfo>& from_selection_data_map,
                        std::map<OPENDCC_NAMESPACE::AnimEngine::CurveId, SplineWidget::CurveData>& to_widget_curve_map);
QColor color_for_component(uint32_t component_idx);

OPENDCC_NAMESPACE_CLOSE
