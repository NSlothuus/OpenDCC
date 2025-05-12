/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/sdf/path.h>
#include "opendcc/app/core/api.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API PointCloudBVH
{
public:
    PointCloudBVH();
    ~PointCloudBVH();
    int get_nearest_point(const PXR_NS::GfVec3f& point, const PXR_NS::SdfPath& prim_path = PXR_NS::SdfPath(),
                          float max_radius = std::numeric_limits<float>::max());
    std::vector<int> get_points_in_radius(const PXR_NS::GfVec3f& point, const PXR_NS::SdfPath& prim_path, float radius);

    bool has_prim(const PXR_NS::SdfPath& prim_path) const;
    void add_prim(const PXR_NS::SdfPath& prim_path, const PXR_NS::GfMatrix4d& world, const PXR_NS::VtVec3fArray& points,
                  const PXR_NS::VtIntArray& indices = PXR_NS::VtIntArray());
    void remove_prim(const PXR_NS::SdfPath& prim_path);
    void set_prim_transform(const PXR_NS::SdfPath& prim_path, const PXR_NS::GfMatrix4d& world_transform);
    void clear();

private:
    struct PointCloudData;
    PointCloudData* m_data;
};
OPENDCC_NAMESPACE_CLOSE
