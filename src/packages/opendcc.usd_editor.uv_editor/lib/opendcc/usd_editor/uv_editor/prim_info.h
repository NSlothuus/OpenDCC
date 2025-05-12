/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/selection_list.h"

#include <vector>

#include <pxr/imaging/hd/meshTopology.h>
#include <pxr/base/vt/array.h>
#include <pxr/base/vt/value.h>
#include <pxr/base/gf/range3d.h>

OPENDCC_NAMESPACE_OPEN

struct PrimInfo
{
    PXR_NS::GfRange3d range;

    std::vector<int> uv_vertices_to_mesh_vertices;
    std::vector<PXR_NS::VtIntArray> mesh_vertices_to_uv_vertices;

    std::vector<PXR_NS::VtIntArray> uv_edges_to_mesh_edges;
    std::vector<PXR_NS::VtIntArray> mesh_edges_to_uv_edges;

    PXR_NS::HdMeshTopology topology;

    friend bool operator==(const PrimInfo& left, const PrimInfo& right);
};
using PrimInfoMap = std::unordered_map<PXR_NS::SdfPath, PrimInfo, PXR_NS::TfHash>;

SelectionList uv_to_mesh(const SelectionList& selection, const PrimInfoMap& map);
SelectionList mesh_to_uv(const SelectionList& selection, const PrimInfoMap& map);

OPENDCC_NAMESPACE_CLOSE

PXR_NAMESPACE_OPEN_SCOPE

template <>
size_t VtHashValue<OPENDCC_NAMESPACE::PrimInfoMap>(const OPENDCC_NAMESPACE::PrimInfoMap& prim_info_map);

PXR_NAMESPACE_CLOSE_SCOPE
