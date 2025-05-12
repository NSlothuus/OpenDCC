// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "hd_selection_ext.h"
#include "pxr/base/tf/debug.h"
#include "pxr/base/tf/diagnostic.h"
#include "pxr/imaging/hd/debugCodes.h"
#include <algorithm>
#include <numeric>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

void HdSelectionExt::add_points(HighlightMode mode, const SdfPath& path, const std::vector<VtIntArray>& point_indices,
                                const VtVec4fArray& point_colors)
{
    VtIntArray point_color_ids(point_colors.size());
    std::iota(point_color_ids.begin(), point_color_ids.end(), _selectedPointColors.size());
    _selectedPointColors.insert(_selectedPointColors.end(), point_colors.begin(), point_colors.end());
    add_points_helper(mode, path, point_indices, point_color_ids);
}

void HdSelectionExt::add_points_helper(HighlightMode mode, const SdfPath& path, const std::vector<VtIntArray>& point_indices,
                                       const VtIntArray& point_color_inds)
{
    if (!point_indices.empty())
    {
        auto& cur_point_indices = _selMap[mode][path].pointIndices;
        auto& point_color_indices = _selMap[mode][path].pointColorIndices;
        cur_point_indices.insert(cur_point_indices.end(), point_indices.begin(), point_indices.end());
        point_color_indices.insert(point_color_indices.end(), point_color_inds.begin(), point_color_inds.end());
    }
}

OPENDCC_NAMESPACE_CLOSE