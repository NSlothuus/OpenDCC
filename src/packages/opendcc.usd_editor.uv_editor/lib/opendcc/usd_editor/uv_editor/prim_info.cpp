// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/uv_editor/prim_info.h"

OPENDCC_NAMESPACE_OPEN

bool operator==(const PrimInfo& left, const PrimInfo& right)
{
    return left.range == right.range && left.mesh_vertices_to_uv_vertices == right.mesh_vertices_to_uv_vertices &&
           left.uv_vertices_to_mesh_vertices == right.uv_vertices_to_mesh_vertices && left.mesh_edges_to_uv_edges == right.mesh_edges_to_uv_edges &&
           left.uv_edges_to_mesh_edges == right.uv_edges_to_mesh_edges && left.topology == right.topology;
}

SelectionList uv_to_mesh(const SelectionList& selection, const PrimInfoMap& map)
{
    SelectionList result;

    for (const auto& select : selection)
    {
        const auto path = select.first;
        const auto find = map.find(select.first);
        if (find == map.end())
        {
            continue;
        }
        const auto& prim_info = *find;

        const auto points = select.second.get_point_indices();
        if (!points.empty())
        {
            std::vector<int> mesh_points;
            mesh_points.reserve(points.size());

            for (const auto point : points)
            {
                mesh_points.push_back(prim_info.second.uv_vertices_to_mesh_vertices[point]);
            }

            result.add_points(path, mesh_points);
        }

        const auto edges = select.second.get_edge_indices();
        if (!edges.empty())
        {
            std::vector<int> mesh_edges;
            mesh_edges.reserve(edges.size());

            for (const auto edge : edges)
            {
                const auto mesh_edges_by_uv = prim_info.second.uv_edges_to_mesh_edges[edge];
                mesh_edges.insert(mesh_edges.end(), mesh_edges_by_uv.begin(), mesh_edges_by_uv.end());
            }

            result.add_edges(path, mesh_edges);
        }

        const auto faces = select.second.get_element_index_intervals();
        if (!faces.empty())
        {
            result.add_elements(path, faces.flatten<std::vector<int>>());
        }
    }

    return result;
}

SelectionList mesh_to_uv(const SelectionList& selection, const PrimInfoMap& map)
{
    SelectionList result;

    for (const auto& select : selection)
    {
        const auto path = select.first;
        const auto find = map.find(select.first);
        if (find == map.end())
        {
            continue;
        }
        const auto& prim_info = *find;

        const auto points = select.second.get_point_indices();
        if (!points.empty())
        {
            std::vector<int> mesh_points;
            mesh_points.reserve(points.size());

            for (const auto point : points)
            {
                const auto mesh_vertices_by_uv = prim_info.second.mesh_vertices_to_uv_vertices[point];
                mesh_points.insert(mesh_points.end(), mesh_vertices_by_uv.begin(), mesh_vertices_by_uv.end());
            }

            result.add_points(path, mesh_points);
        }

        const auto edges = select.second.get_edge_indices();
        if (!edges.empty())
        {
            std::vector<int> mesh_edges;
            mesh_edges.reserve(edges.size());

            for (const auto edge : edges)
            {
                const auto mesh_edges_by_uv = prim_info.second.mesh_edges_to_uv_edges[edge];
                mesh_edges.insert(mesh_edges.end(), mesh_edges_by_uv.begin(), mesh_edges_by_uv.end());
            }

            result.add_edges(path, mesh_edges);
        }

        const auto faces = select.second.get_element_index_intervals();
        if (!faces.empty())
        {
            result.add_elements(path, faces.flatten<std::vector<int>>());
        }
    }

    return result;
}

OPENDCC_NAMESPACE_CLOSE

PXR_NAMESPACE_OPEN_SCOPE

template <>
size_t VtHashValue<OPENDCC_NAMESPACE::PrimInfoMap>(const OPENDCC_NAMESPACE::PrimInfoMap& prim_info_map)
{
    size_t result = 0;

    for (const auto& prim_info : prim_info_map)
    {
        result += prim_info.second.topology.ComputeHash();
    }

    return result;
}

PXR_NAMESPACE_CLOSE_SCOPE
