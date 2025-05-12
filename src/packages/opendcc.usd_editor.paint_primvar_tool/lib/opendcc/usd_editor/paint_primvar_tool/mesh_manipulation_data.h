/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
// workaround
#include "opendcc/base/qt_python.h"
#include <pxr/pxr.h>
#include "pxr/imaging/hd/meshTopology.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "opendcc/app/core/point_cloud_bvh.h"
#include "pxr/imaging/hd/smoothNormals.h"
#include "pxr/imaging/hd/vertexAdjacency.h"
#include "opendcc/app/core/undo/block.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

float falloff_function(float falloff, float normalize_radius);

struct MeshManipulationData
{
    UsdGeomMesh mesh;
    std::vector<float> scales;
    PointCloudBVH bvh;
    VtVec3fArray points;
    VtVec3fArray normals;
    Hd_VertexAdjacency adjacency;
    std::unique_ptr<commands::UsdEditsUndoBlock> undo_block;
    const static float empty_scale;
    MeshManipulationData(const UsdGeomMesh& in_mesh, bool& success);
};

OPENDCC_NAMESPACE_CLOSE
