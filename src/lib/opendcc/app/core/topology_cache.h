/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <pxr/usd/sdf/path.h>
#include <pxr/imaging/hd/meshTopology.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/imaging/hd/meshUtil.h>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API EdgeIndexTable
{
public:
    EdgeIndexTable(const PXR_NS::HdMeshTopology* topology);

    std::tuple<PXR_NS::GfVec2i, bool> get_vertices_by_edge_id(int edge_id) const;
    std::tuple<std::vector<int>, bool> get_edge_id_by_edge_vertices(const PXR_NS::GfVec2i& edge_vertices) const;
    size_t get_edge_count() const { return m_edge_vertices.size(); }

private:
    struct Edge
    {
        Edge(const PXR_NS::GfVec2i& vertices = PXR_NS::GfVec2i(-1), int index = -1)
            : verts(vertices)
            , id(index)
        {
            if (verts[0] > verts[1])
                std::swap(verts[0], verts[1]);
        }
        PXR_NS::GfVec2i verts;
        int id;
    };

    struct EdgeLessThan
    {
        bool operator()(const Edge& l, const Edge& r) const
        {
            return l.verts[0] < r.verts[0] || (l.verts[0] == r.verts[0] && l.verts[1] < r.verts[1]);
        }
    };

    std::vector<PXR_NS::GfVec2i> m_edge_vertices;
    std::vector<Edge> m_index_to_edge;
};

class OPENDCC_API TopologyCache
{
public:
    struct Topology
    {
        PXR_NS::HdMeshTopology mesh_topology;
        EdgeIndexTable edge_map;
        PXR_NS::VtIntArray face_starts;
    };
    using TopologySharedPtr = std::shared_ptr<const Topology>;

    TopologyCache() = default;

    void clear_at_time(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time_code);
    void clear_all();
    void clear_all_timesamples(const PXR_NS::UsdPrim& prim);
    TopologySharedPtr get_topology(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time_code = PXR_NS::UsdTimeCode::Default());

private:
    using MeshSamples = std::unordered_map<PXR_NS::UsdTimeCode, TopologySharedPtr, PXR_NS::TfHash>;
    using PerPrimCache = std::unordered_map<PXR_NS::UsdPrim, MeshSamples, PXR_NS::TfHash>;

    PerPrimCache m_per_prim_cache;
};
OPENDCC_NAMESPACE_CLOSE
