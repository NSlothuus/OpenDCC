// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_volume_locator.h"
#include <mutex>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE;

VolumeLocatorRenderData::VolumeLocatorRenderData()
{
    m_points.resize(8);
}

void VolumeLocatorRenderData::update(const std::unordered_map<std::string, PXR_NS::VtValue>& data)
{
    auto extent_iter = data.find("extent");
    if (extent_iter != data.end())
    {
        const auto& extent = extent_iter->second.Get<VtVec3fArray>();
        m_bbox = { extent[0], extent[1] };
    }
    update_points();
}

const PXR_NS::VtArray<int>& VolumeLocatorRenderData::vertex_per_curve() const
{
    static const VtArray<int> vpc = { 4, 4, 4, 4, 4, 4 };
    return vpc;
}

const PXR_NS::VtArray<int>& VolumeLocatorRenderData::vertex_indexes() const
{
    static const VtArray<int> indexes = { 3, 2, 0, 1, 2, 6, 4, 0, 4, 5, 7, 6, 5, 1, 3, 7, 4, 5, 1, 0, 2, 3, 7, 6 };
    return indexes;
}

const PXR_NS::VtVec3fArray& VolumeLocatorRenderData::vertex_positions() const
{
    return m_points;
}

const PXR_NS::GfRange3d& VolumeLocatorRenderData::bbox() const
{
    return m_bbox;
}

const PXR_NS::TfToken& VolumeLocatorRenderData::topology() const
{
    return HdTokens->periodic;
}

const bool VolumeLocatorRenderData::as_mesh() const
{
    return false;
}

const bool VolumeLocatorRenderData::is_double_sided() const
{
    return true;
}

void VolumeLocatorRenderData::update_points()
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    for (int i = 0; i < m_points.size(); i++)
    {
        m_points[i] = GfVec3f(m_bbox.GetCorner(i));
    }
}

OPENDCC_NAMESPACE_CLOSE