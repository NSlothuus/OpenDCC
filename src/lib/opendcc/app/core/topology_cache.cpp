// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/topology_cache.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/imaging/hd/meshTopology.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

template <class TResult>
TResult get_attr_value(const UsdPrim& prim, const TfToken& attr, UsdTimeCode time)
{
    TResult result;
    prim.GetAttribute(attr).Get<TResult>(&result, time);
    return result;
}

void TopologyCache::clear_at_time(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time_code)
{
    auto iter = m_per_prim_cache.find(prim);
    if (iter == m_per_prim_cache.end())
        return;

    iter->second.erase(time_code.GetValue());
    if (iter->second.empty())
        m_per_prim_cache.erase(iter);
}

void TopologyCache::clear_all()
{
    m_per_prim_cache.clear();
}

void TopologyCache::clear_all_timesamples(const PXR_NS::UsdPrim& prim)
{
    m_per_prim_cache.erase(prim);
}

TopologyCache::TopologySharedPtr TopologyCache::get_topology(const PXR_NS::UsdPrim& prim,
                                                             PXR_NS::UsdTimeCode time_code /*= PXR_NS::UsdTimeCode::Default()*/)
{
    UsdGeomMesh mesh = UsdGeomMesh(prim);
    if (!mesh)
        return nullptr;

    auto mesh_samples = TfMapLookupPtr(m_per_prim_cache, prim);
    if (!mesh_samples)
    {
        mesh_samples = &(m_per_prim_cache.insert({ prim, MeshSamples() }).first->second);
    }

    auto topology_ptr = TfMapLookupByValue(*mesh_samples, time_code, TopologySharedPtr());
    if (!topology_ptr)
    {
        auto mesh_topology = HdMeshTopology(get_attr_value<TfToken>(prim, UsdGeomTokens->subdivisionScheme, time_code),
                                            get_attr_value<TfToken>(prim, UsdGeomTokens->orientation, time_code),
                                            get_attr_value<VtIntArray>(prim, UsdGeomTokens->faceVertexCounts, time_code),
                                            get_attr_value<VtIntArray>(prim, UsdGeomTokens->faceVertexIndices, time_code),
                                            get_attr_value<VtIntArray>(prim, UsdGeomTokens->holeIndices, time_code));

        EdgeIndexTable edge_map(&mesh_topology);
        const auto& face_vertex_counts = mesh_topology.GetFaceVertexCounts();
        VtIntArray face_starts(face_vertex_counts.size());
        size_t offset = 0;
        for (int face_id = 0; face_id < face_vertex_counts.size(); face_id++)
        {
            face_starts[face_id] = offset;
            offset += face_vertex_counts[face_id];
        }

        topology_ptr = std::make_shared<Topology>(Topology { mesh_topology, edge_map, face_starts });
        mesh_samples->insert({ time_code, topology_ptr });
    }
    return topology_ptr;
}

EdgeIndexTable::EdgeIndexTable(const PXR_NS::HdMeshTopology* topology)
{
#if PXR_VERSION < 2108
    auto verts_to_edge_id = HdMeshUtil::ComputeAuthoredEdgeMap(topology);
    m_edge_vertices.resize(verts_to_edge_id.size());
    m_index_to_edge.resize(verts_to_edge_id.size());
    for (auto& edge : verts_to_edge_id)
    {
        auto edge_val = edge.first;
        if (edge.first[0] > edge.first[1])
            std::swap(edge_val[0], edge_val[1]);
        m_edge_vertices[edge.second] = edge_val;
        m_index_to_edge[edge.second] = Edge(edge_val, edge.second);
    }
#else
    HdMeshUtil mesh_util(topology, SdfPath());
    mesh_util.EnumerateEdges(&m_edge_vertices);
    m_index_to_edge.resize(m_edge_vertices.size());
    for (size_t i = 0; i < m_edge_vertices.size(); i++)
        m_index_to_edge[i] = Edge(m_edge_vertices[i], i);
#endif
    std::sort(m_index_to_edge.begin(), m_index_to_edge.end(), EdgeLessThan());
}

std::tuple<GfVec2i, bool> EdgeIndexTable::get_vertices_by_edge_id(int edge_id) const
{
    if (edge_id < 0 || static_cast<size_t>(edge_id) >= m_edge_vertices.size())
        return { GfVec2i(), false };

    return { m_edge_vertices[edge_id], true };
}

std::tuple<std::vector<int>, bool> EdgeIndexTable::get_edge_id_by_edge_vertices(const GfVec2i& edge_vertices) const
{
    const auto match = std::equal_range(m_index_to_edge.begin(), m_index_to_edge.end(), Edge(edge_vertices), EdgeLessThan());
    if (match.first == match.second)
        return { std::vector<int>(), false };

    std::vector<int> result(std::distance(match.first, match.second));
    std::transform(match.first, match.second, result.begin(), [](const Edge& edge) { return edge.id; });
    return { result, true };
}
OPENDCC_NAMESPACE_CLOSE
