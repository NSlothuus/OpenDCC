/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/viewport/viewport_move_snap_strategy.h"
#include "opendcc/app/core/selection_list.h"
#include "opendcc/app/viewport/viewport_view.h"

OPENDCC_NAMESPACE_OPEN

class ViewportUsdMeshScreenSnapStrategy : public ViewportSnapStrategy
{
public:
    ViewportUsdMeshScreenSnapStrategy(const SelectionList& selection);

    void set_viewport_data(const ViewportViewPtr& viewport_view, const PXR_NS::GfVec2f& screen_point,
                           PXR_NS::UsdTimeCode = PXR_NS::UsdTimeCode::Default());

protected:
    bool is_valid_snap_state() const;
    PXR_NS::GfVec3d get_fallback_snap_value(const PXR_NS::GfVec3d& start_pos, const PXR_NS::GfVec3d& start_drag,
                                            const PXR_NS::GfVec3d& cur_drag) const;

    SelectionList m_selection_list;
    PXR_NS::GfMatrix4d m_view_proj;
    ViewportViewPtr m_viewport_view;
    PXR_NS::GfVec2f m_screen_point;
    PXR_NS::UsdTimeCode m_time;
};

class ViewportUsdVertexScreenSnapStrategy : public ViewportUsdMeshScreenSnapStrategy
{
public:
    ViewportUsdVertexScreenSnapStrategy(const SelectionList& selection);
    PXR_NS::GfVec3d get_snap_point(const PXR_NS::GfVec3d& start_pos, const PXR_NS::GfVec3d& start_drag, const PXR_NS::GfVec3d& cur_drag) override;
};

class ViewportUsdEdgeScreenSnapStrategy : public ViewportUsdMeshScreenSnapStrategy
{
public:
    ViewportUsdEdgeScreenSnapStrategy(const SelectionList& selection, bool to_center);
    PXR_NS::GfVec3d get_snap_point(const PXR_NS::GfVec3d& start_pos, const PXR_NS::GfVec3d& start_drag, const PXR_NS::GfVec3d& cur_drag) override;

private:
    bool m_to_center = false;
};

class ViewportUsdFaceScreenSnapStrategy : public ViewportUsdMeshScreenSnapStrategy
{
public:
    ViewportUsdFaceScreenSnapStrategy(const SelectionList& selection, bool to_center);
    PXR_NS::GfVec3d get_snap_point(const PXR_NS::GfVec3d& start_pos, const PXR_NS::GfVec3d& start_drag, const PXR_NS::GfVec3d& cur_drag) override;

private:
    bool m_to_center = false;
};
OPENDCC_NAMESPACE_CLOSE