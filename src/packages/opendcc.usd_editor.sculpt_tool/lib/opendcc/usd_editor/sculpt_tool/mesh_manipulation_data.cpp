// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/sculpt_tool/mesh_manipulation_data.h"
#include "opendcc/app/core/application.h"
#include "opendcc/usd_editor/sculpt_tool/utils.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

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

bool MeshManipulationData::update_geometry()
{
    if (!mesh.GetPointsAttr().Get(&initial_world_points) || initial_world_points.size() == 0)
        return false;
    UsdTimeCode time = UsdTimeCode::Default();
    UsdGeomXformCache xform_cache(time);
    GfMatrix4d local2world = xform_cache.GetLocalToWorldTransform(mesh.GetPrim());
    auto points_data = initial_world_points.data();
    for (size_t i = 0; i < initial_world_points.size(); ++i)
        points_data[i] = GfVec3f(local2world.Transform(points_data[i]));

    initial_world_normals = Hd_SmoothNormals::ComputeSmoothNormals(&adjacency, initial_world_points.size(), initial_world_points.cdata());
    return mesh_bvh.update_geometry();
}

MeshManipulationData::MeshManipulationData(const UsdGeomMesh& in_mesh, bool& success)
{
    success = false;
    auto time_code = Application::instance().get_current_time();
    mesh = in_mesh;
    if (!mesh.GetPrim().IsValid() || !mesh.GetPointsAttr().Get(&initial_world_points) || initial_world_points.size() == 0)
        return;

    UsdTimeCode time = UsdTimeCode::Default();
    UsdGeomXformCache xform_cache(time);
    GfMatrix4d local2world = xform_cache.GetLocalToWorldTransform(mesh.GetPrim());
    auto points_data = initial_world_points.data();
    for (size_t i = 0; i < initial_world_points.size(); ++i)
        points_data[i] = GfVec3f(local2world.Transform(points_data[i]));

    VtIntArray face_vertex_counts;
    bool ok = mesh.GetFaceVertexCountsAttr().Get(&face_vertex_counts, time_code);
    if (!ok || face_vertex_counts.size() == 0)
        return;

    VtIntArray face_vertex_indices;
    ok = mesh.GetFaceVertexIndicesAttr().Get(&face_vertex_indices, time_code);
    if (!ok || face_vertex_indices.size() == 0)
        return;

    VtIntArray hole_indices;
    ok = mesh.GetHoleIndicesAttr().Get(&hole_indices, time_code);
    if (!ok)
        return;

    auto mesh_topology = HdMeshTopology(get_attr_value<TfToken>(mesh.GetPrim(), UsdGeomTokens->subdivisionScheme, time_code),
                                        get_attr_value<TfToken>(mesh.GetPrim(), UsdGeomTokens->orientation, time_code), face_vertex_counts,
                                        face_vertex_indices, hole_indices);

    adjacency.BuildAdjacencyTable(&mesh_topology);
    initial_world_normals = Hd_SmoothNormals::ComputeSmoothNormals(&adjacency, initial_world_points.size(), initial_world_points.cdata());
    mesh_bvh.set_prim(mesh.GetPrim());

    if (!mesh_bvh.is_valid())
    {
        success = false;
        return;
    }

    success = true;
}

void MeshManipulationData::update_extent()
{
    if (!mesh)
        return;
    VtVec3fArray extent;
    VtArray<GfVec3f> points;
    mesh.GetPointsAttr().Get(&points);
    if (UsdGeomPointBased::ComputeExtent(points, &extent))
    {
        PXR_NS::UsdGeomPointBased points_base(mesh);
        if (points_base)
            points_base.GetExtentAttr().Set(extent);
    }
}

UndoMeshManipulationData::UndoMeshManipulationData(const UsdGeomMesh& in_mesh, bool& success)
    : MeshManipulationData(in_mesh, success)
{
}

void UndoMeshManipulationData::on_finish()
{
    if (undo_block)
    {
        update_extent();
        undo_block.reset();
    }
}

OPENDCC_NAMESPACE_CLOSE
