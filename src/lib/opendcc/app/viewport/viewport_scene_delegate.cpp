// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_scene_delegate.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<ViewportSceneDelegate>();
}

ViewportSceneDelegate::ViewportSceneDelegate(PXR_NS::HdRenderIndex* render_index, const PXR_NS::SdfPath& delegate_id)
    : HdSceneDelegate(render_index, delegate_id)
{
}

void ViewportSceneDelegate::set_selection_mode(PXR_NS::HdSelection::HighlightMode selection_mode)
{
    m_selection_mode = selection_mode;
}

SdfPath ViewportSceneDelegate::convert_stage_path_to_index_path(const SdfPath& stage_path) const
{
    return stage_path.ReplacePrefix(SdfPath::AbsoluteRootPath(), GetDelegateID());
}

SdfPath ViewportSceneDelegate::convert_index_path_to_stage_path(const SdfPath& index_path) const
{
    return index_path.ReplacePrefix(GetDelegateID(), SdfPath::AbsoluteRootPath());
}
OPENDCC_NAMESPACE_CLOSE
