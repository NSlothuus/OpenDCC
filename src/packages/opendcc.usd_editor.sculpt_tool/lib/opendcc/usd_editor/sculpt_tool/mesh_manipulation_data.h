/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include <pxr/pxr.h>
#include <pxr/imaging/hd/meshTopology.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/imaging/hd/smoothNormals.h>
#include <pxr/imaging/hd/vertexAdjacency.h>

#include "opendcc/base/qt_python.h"
#include "opendcc/app/core/mesh_bvh.h"
#include "opendcc/app/core/undo/block.h"

OPENDCC_NAMESPACE_OPEN

struct MeshManipulationData
{
    PXR_NS::UsdGeomMesh mesh;
    MeshBvh mesh_bvh;
    PXR_NS::VtVec3fArray initial_world_points;
    PXR_NS::VtVec3fArray initial_world_normals;
    PXR_NS::Hd_VertexAdjacency adjacency;
    bool update_geometry();
    MeshManipulationData(const PXR_NS::UsdGeomMesh& in_mesh, bool& success);
    void update_extent();
};

struct UndoMeshManipulationData : public MeshManipulationData
{
    UndoMeshManipulationData(const PXR_NS::UsdGeomMesh& in_mesh, bool& success);

    void on_finish();

    std::unique_ptr<commands::UsdEditsUndoBlock> undo_block;
};

OPENDCC_NAMESPACE_CLOSE
