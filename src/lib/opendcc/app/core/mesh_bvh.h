/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <memory>
#include "pxr/usd/usd/prim.h"
#include "opendcc/app/core/api.h"

OPENDCC_NAMESPACE_OPEN
class OPENDCC_API MeshBvh
{
public:
    MeshBvh();
    MeshBvh(const PXR_NS::UsdPrim& prim);
    ~MeshBvh();
    void set_prim(const PXR_NS::UsdPrim& prim);
    bool cast_ray(PXR_NS::GfVec3f origin, PXR_NS::GfVec3f dir, PXR_NS::GfVec3f& hit_point, PXR_NS::GfVec3f& hit_normal) const;
    bool is_valid() const;
    bool update_geometry();
    std::vector<int> get_points_in_radius(const PXR_NS::GfVec3f& point, const PXR_NS::SdfPath& prim_path, float radius);

private:
    struct MeshBvhImpl;
    std::unique_ptr<MeshBvhImpl> m_impl;
};

OPENDCC_NAMESPACE_CLOSE
