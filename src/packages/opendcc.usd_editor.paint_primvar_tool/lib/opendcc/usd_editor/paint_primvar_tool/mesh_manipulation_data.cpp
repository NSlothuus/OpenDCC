// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/paint_primvar_tool/mesh_manipulation_data.h"
#include "opendcc/app/core/application.h"

OPENDCC_NAMESPACE_OPEN

const float MeshManipulationData::empty_scale = -1e10f;

float falloff_function(float falloff, float normalize_radius)
{
    if (normalize_radius > 1)
        return 0;
    else if (falloff < 0.05)
        return 1.0;
    else if (falloff > 0.51)
        return (1 - normalize_radius) * std::exp(-(falloff - 0.5f) * 10 * normalize_radius);
    else if (falloff < 0.49)
        return (1 - normalize_radius * normalize_radius) * (1 - std::exp((falloff - 0.5f) * 30 * (1 - normalize_radius)));
    else
        return 1 - normalize_radius;
};
namespace
{
    template <class TResult>
    TResult get_attr_value(const UsdPrim& prim, const TfToken& attr, UsdTimeCode time)
    {
        TResult result;
        prim.GetAttribute(attr).Get<TResult>(&result, time);
        return result;
    }
}

MeshManipulationData::MeshManipulationData(const UsdGeomMesh& in_mesh, bool& success)
{
    success = false;
    auto time_code = Application::instance().get_current_time();
    mesh = in_mesh;
    if (!mesh.GetPointsAttr().Get(&points) || points.size() == 0)
        return;

    auto mesh_topology = HdMeshTopology(get_attr_value<TfToken>(mesh.GetPrim(), UsdGeomTokens->subdivisionScheme, time_code),
                                        get_attr_value<TfToken>(mesh.GetPrim(), UsdGeomTokens->orientation, time_code),
                                        get_attr_value<VtIntArray>(mesh.GetPrim(), UsdGeomTokens->faceVertexCounts, time_code),
                                        get_attr_value<VtIntArray>(mesh.GetPrim(), UsdGeomTokens->faceVertexIndices, time_code),
                                        get_attr_value<VtIntArray>(mesh.GetPrim(), UsdGeomTokens->holeIndices, time_code));

    adjacency.BuildAdjacencyTable(&mesh_topology);
    normals = Hd_SmoothNormals::ComputeSmoothNormals(&adjacency, points.size(), points.cdata());
    UsdGeomXformCache xform_cache(Application::instance().get_current_time());
    bvh.add_prim(mesh.GetPrim().GetPath(), xform_cache.GetLocalToWorldTransform(mesh.GetPrim()), points);
    scales.resize(points.size(), empty_scale);
    success = true;
}

OPENDCC_NAMESPACE_CLOSE
