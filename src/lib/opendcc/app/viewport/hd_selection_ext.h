/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/imaging/hd/selection.h>

OPENDCC_NAMESPACE_OPEN

class HdSelectionExt : public PXR_NS::HdSelection
{
public:
    void add_points(HighlightMode mode, const PXR_NS::SdfPath& path, const std::vector<PXR_NS::VtIntArray>& point_indices,
                    const PXR_NS::VtVec4fArray& point_colors);

private:
    void add_points_helper(HighlightMode mode, const PXR_NS::SdfPath& path, const std::vector<PXR_NS::VtIntArray>& point_indices,
                           const PXR_NS::VtIntArray& point_color_inds);
};
OPENDCC_NAMESPACE_CLOSE