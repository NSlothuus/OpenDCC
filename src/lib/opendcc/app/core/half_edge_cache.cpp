// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/half_edge_cache.h"

#include "opendcc/app/core/application.h"

#include <queue>
#include <vector>

#define _USE_MATH_DEFINES
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>
#include <OpenMesh/Core/Utils/Predicates.hh>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

//////////////////////////////////////////////////////////////////////////
// OpenMesh
//////////////////////////////////////////////////////////////////////////

using OMHalfEdge = OpenMesh::PolyMesh_ArrayKernelT<>;

static bool is_border(const OpenMesh::SmartHalfedgeHandle& outgoint)
{
    return !outgoint.face().is_valid() || !outgoint.opp().face().is_valid();
}

namespace loop_selection
{
    static bool can_select(const OpenMesh::SmartHalfedgeHandle& to_select, const OpenMesh::SmartHalfedgeHandle& prev_select)
    {
        return to_select.is_valid() && to_select.face() != prev_select.face();
    };

    static OpenMesh::SmartHalfedgeHandle find_next_half_edge(const OpenMesh::PolyConnectivity::ConstVertexOHalfedgeRange& range,
                                                             const OpenMesh::SmartHalfedgeHandle& selected, const uint valence)
    {
        const auto begin = range.begin();
        const auto end = range.end();
        auto find = std::find(begin, end, selected);
        if (find == end)
        {
            return OpenMesh::SmartHalfedgeHandle();
        }

        if (is_border(selected))
        {
            int steps = valence;
            while (steps--)
            {
                ++find;
                if (find == end)
                {
                    find = begin;
                }

                if (is_border(*find) && *find != selected)
                {
                    break;
                }
            }
        }
        else
        {
            int steps = valence / 2;
            while (steps--)
            {
                ++find;
                if (find == end)
                {
                    find = begin;
                }
            }
        }

        return *find == selected ? OpenMesh::SmartHalfedgeHandle() : *find;
    }
}

//////////////////////////////////////////////////////////////////////////
// HalfEdge::HalfEdgeImpl
//////////////////////////////////////////////////////////////////////////

class HalfEdge::HalfEdgeImpl
{
    constexpr static int s_invalid_topoly_id = -1;

public:
    HalfEdgeImpl()
        : m_edge_indices(m_half_edge, "edge_indices")
        ,
        // m_edge_topology_id(m_half_edge, "topology_id"),
        m_face_topology_id(m_half_edge, "topology_id")
        , m_vertex_topology_id(m_half_edge, "topology_id")
    {
    }

    ~HalfEdgeImpl()
    {
        if (m_selection_changed)
        {
            Application::instance().unregister_event_callback(Application::EventType::SELECTION_CHANGED, m_selection_changed);
        }
    }

    static HalfEdgeImplPtr from_mesh(const PXR_NS::UsdGeomMesh& mesh, PXR_NS::UsdTimeCode time)
    {
        if (!mesh)
        {
            return nullptr;
        }

        const auto session = Application::instance().get_session();
        if (!session)
        {
            return nullptr;
        }

        const auto stage = session->get_current_stage_id();
        if (!stage)
        {
            return nullptr;
        }

        auto cache = session->get_stage_topology_cache(stage);
        auto topology = cache.get_topology(mesh.GetPrim(), time);
        auto edge_index_table = EdgeIndexTable(&topology->mesh_topology);

        std::vector<double> times;
        if (!mesh.GetPointsAttr().GetTimeSamples(&times))
        {
            return nullptr;
        }

        // If the time is equal to the PXR_NS::UsdTimeCode::Default() and the PointsAttr is animated,
        // calling mesh.GetPointsAttr().Get(&points, PXR_NS::UsdTimeCode::Default()) will return an invalid result.
        // However, the animation of the PointsAttr does not affect the topology.
        const auto correct_time = !times.empty() && time == PXR_NS::UsdTimeCode::Default() ? times[0] : time;

        VtVec3fArray points;
        if (!mesh.GetPointsAttr().Get(&points, correct_time) || points.empty())
        {
            return nullptr;
        }

        HalfEdgeImplPtr result = std::make_unique<HalfEdgeImpl>();
        result->m_half_edge.request_vertex_status();
        result->m_half_edge.request_edge_status();
        result->m_half_edge.request_face_status();

        std::vector<OMHalfEdge::VertexHandle> vertex_handles;
        vertex_handles.reserve(points.size());
        for (const auto point : points)
        {
            vertex_handles.push_back(result->m_half_edge.add_vertex(OMHalfEdge::Point(point[0], point[1], point[2])));
        }

        const auto& face_vertex_counts = topology->mesh_topology.GetFaceVertexCounts();
        const auto& face_vertex_indices = topology->mesh_topology.GetFaceVertexIndices();
        const auto face_vertex_count_size = face_vertex_counts.size();
        bool const flip = (topology->mesh_topology.GetOrientation() != HdTokens->rightHanded);

        std::vector<OMHalfEdge::VertexHandle> face_handles;
        auto offset = 0;
        for (const auto count : face_vertex_counts)
        {
            for (int i = offset; i < offset + count; ++i)
            {
                const auto vertex_index = face_vertex_indices[i];
                face_handles.push_back(vertex_handles[vertex_index]);
            }
            result->m_half_edge.add_face(face_handles);
            face_handles.clear();
            offset += count;
        }

        result->m_half_edge.vertices().for_each(
            [&](const typename OpenMesh::SmartVertexHandle& vertex) { result->m_vertex_topology_id[vertex] = s_invalid_topoly_id; });
        result->m_half_edge.edges().for_each([&](const OpenMesh::SmartEdgeHandle& edge) {
            const auto edge_index_tuple = edge_index_table.get_edge_id_by_edge_vertices({ edge.v0().idx(), edge.v1().idx() });
            if (std::get<bool>(edge_index_tuple))
            {
                result->m_edge_indices[edge] = std::get<std::vector<int>>(edge_index_tuple);
            }
            // result->m_edge_topology_id[edge] = s_invalid_topoly_id;
        });
        result->m_half_edge.faces().for_each(
            [&](const typename OpenMesh::SmartFaceHandle& face) { result->m_face_topology_id[face] = s_invalid_topoly_id; });

        int topology_id = 0;
        auto find = result->m_half_edge.vertices_begin();
        while (true)
        {
            find = std::find_if(find, result->m_half_edge.vertices_end(), [&](const typename OpenMesh::VertexHandle& vertex_handle) {
                return result->m_vertex_topology_id[vertex_handle] == s_invalid_topoly_id;
            });

            if (find == result->m_half_edge.vertices_end())
            {
                break;
            }

            std::queue<OpenMesh::SmartVertexHandle> queue;
            queue.push(*find);
            result->m_vertex_topology_id[*find] = topology_id;

            while (!queue.empty())
            {
                const auto front = queue.front();

                front.vertices().for_each([&](const typename OpenMesh::SmartVertexHandle& vertex) {
                    if (result->m_vertex_topology_id[vertex] == s_invalid_topoly_id)
                    {
                        result->m_vertex_topology_id[vertex] = topology_id;
                        queue.push(vertex);
                    }
                });
                // front.edges().for_each(
                //     [&](const typename OpenMesh::SmartEdgeHandle& edge)
                //     {
                //         result->m_edge_topology_id[edge] = topology_id;
                //     }
                // );
                front.faces().for_each([&](const typename OpenMesh::SmartFaceHandle& face) { result->m_face_topology_id[face] = topology_id; });

                queue.pop();
            }

            ++topology_id;
        }

        result->m_mesh = mesh;
        return result;
    }

    SelectionList edge_loop_selection(const PXR_NS::GfVec2i& begin)
    {
        const auto half_edge_begin = get_half_edge(begin);
        if (!half_edge_begin.is_valid())
        {
            return SelectionList();
        }

        const auto path = m_mesh.GetPath();

        SelectionList edge_loop_selection;
        std::queue<OpenMesh::SmartHalfedgeHandle> queue;
        queue.push(half_edge_begin);

        const auto is_selected = [&](const OpenMesh::SmartHalfedgeHandle& half_edge) {
            const auto selected_edges = edge_loop_selection.get_selection_data(m_mesh.GetPath()).get_edge_index_intervals();
            const auto edge_indices = m_edge_indices[half_edge.edge()];
            for (const auto edge_index : edge_indices)
            {
                if (selected_edges.contains(edge_index))
                {
                    return true;
                }
            }

            return false;
        };

        const auto try_select = [&](const OpenMesh::SmartVertexHandle& vertex, const OpenMesh::SmartHalfedgeHandle& selected_outgoing) {
            if (!selected_outgoing.is_valid() || !vertex.is_valid())
            {
                return;
            }

            const auto valence = vertex.valence();
            if (valence == 4 || is_border(selected_outgoing))
            {
                const auto outgoing = vertex.outgoing_halfedges();
                auto next = loop_selection::find_next_half_edge(outgoing, selected_outgoing, valence);
                if (loop_selection::can_select(next, selected_outgoing) && !is_selected(next))
                {
                    queue.push(next);
                    edge_loop_selection.add_edges(path, m_edge_indices[next.edge()]);
                }
            }
        };

        while (!queue.empty())
        {
            const auto front = queue.front();

            try_select(front.to(), front.opp());
            try_select(front.from(), front);

            queue.pop();
        }

        return edge_loop_selection;
    }

    SelectionList grow_selection(const SelectionList& current)
    {
        auto& application = Application::instance();
        const auto selection_mode = application.get_selection_mode();

        switch (selection_mode)
        {
        case Application::SelectionMode::POINTS:
        case Application::SelectionMode::UV:
        {
            return select<Application::SelectionMode::POINTS, Type::Grow>(current);
        }
        case Application::SelectionMode::EDGES:
        {
            return select<Application::SelectionMode::EDGES, Type::Grow>(current);
        }
        case Application::SelectionMode::FACES:
        {
            return select<Application::SelectionMode::FACES, Type::Grow>(current);
        }
        }

        return SelectionList();
    }

    SelectionList decrease_selection(const SelectionList& current)
    {
        auto& application = Application::instance();
        const auto selection_mode = application.get_selection_mode();

        switch (selection_mode)
        {
        case Application::SelectionMode::POINTS:
        case Application::SelectionMode::UV:
        {
            return select<Application::SelectionMode::POINTS, Type::Decrease>(current);
        }
        case Application::SelectionMode::EDGES:
        {
            return select<Application::SelectionMode::EDGES, Type::Decrease>(current);
        }
        case Application::SelectionMode::FACES:
        {
            return select<Application::SelectionMode::FACES, Type::Decrease>(current);
        }
        }

        return SelectionList();
    }

    SelectionList topology_selection(const SelectionList& current)
    {
        auto& application = Application::instance();
        const auto selection_mode = application.get_selection_mode();

        const auto path = m_mesh.GetPath();
        const auto selection_data = current[path];

        SelectionList result;

        const auto point_intervals = selection_data.get_point_index_intervals();
        // const auto edge_intervals = selection_data.get_edge_index_intervals();
        const auto element_intervals = selection_data.get_element_index_intervals();

        int topology_id = s_invalid_topoly_id;

        if (!element_intervals.empty())
        {
            const auto edge_indices = selection_data.get_element_indices();
            topology_id = m_face_topology_id[m_half_edge.face_handle(*edge_indices.begin())];
            for (const auto edge : edge_indices)
            {
                if (m_face_topology_id[m_half_edge.face_handle(edge)] != topology_id)
                {
                    return result;
                }
            }
        }
        // else if(!edge_intervals.empty())
        // {
        //     auto& application = Application::instance();
        //     const auto session = application.get_session();
        //     const auto stage = session->get_current_stage();
        //
        //     const auto time = application.get_current_time();
        //     const auto stage_id = session->get_current_stage_id();
        //     auto topology = session->get_stage_topology_cache(stage_id);
        //     auto edge_index_table = EdgeIndexTable(&topology.get_topology(m_mesh.GetPrim(), time)->mesh_topology);
        //
        //     const auto get_topology_id =
        //         [&](const size_t edge_index)
        //         {
        //             const auto edge_indices_tuple = edge_index_table.get_vertices_by_edge_id(edge_index);
        //
        //             if(!std::get<bool>(edge_indices_tuple))
        //             {
        //                 return s_invalid_topoly_id;
        //             }
        //
        //             return m_edge_topology_id[get_half_edge(std::get<GfVec2i>(edge_indices_tuple)).edge()];
        //         };
        //
        //     const auto edge_indices = selection_data.get_edge_indices();
        //     topology_id = get_topology_id(*edge_indices.begin());
        //
        //     for(const auto edge : edge_indices)
        //     {
        //         if(topology_id != get_topology_id(edge))
        //         {
        //             return result;
        //         }
        //     }
        // }
        else if (!point_intervals.empty())
        {
            const auto point_indices = selection_data.get_point_indices();
            topology_id = m_vertex_topology_id[m_half_edge.vertex_handle(*point_indices.begin())];
            for (const auto point : point_indices)
            {
                if (m_vertex_topology_id[m_half_edge.vertex_handle(point)] != topology_id)
                {
                    return result;
                }
            }
        }

        if (topology_id == s_invalid_topoly_id)
        {
            return result;
        }

        switch (selection_mode)
        {
        case Application::SelectionMode::POINTS:
        case Application::SelectionMode::UV:
        {
            const auto vector = m_half_edge.vertices()
                                    .filtered([&](const OpenMesh::SmartVertexHandle& vertex) { return m_vertex_topology_id[vertex] == topology_id; })
                                    .to_vector([&](const OpenMesh::SmartVertexHandle& vertex) { return vertex.idx(); });
            result.add_points(path, vector);
            break;
        }
        // case Application::SelectionMode::EDGES:
        // {
        //     m_half_edge.edges()
        //         .filtered([&](const OpenMesh::SmartEdgeHandle& edge) { return m_edge_topology_id[edge] == topology_id; })
        //         .for_each([&](const OpenMesh::SmartEdgeHandle& edge) { result.add_edges(path, m_edge_indices[edge]); });
        //     break;
        // }
        case Application::SelectionMode::FACES:
        {
            const auto vector = m_half_edge.faces()
                                    .filtered([&](const OpenMesh::SmartFaceHandle& face) { return m_face_topology_id[face] == topology_id; })
                                    .to_vector([&](const OpenMesh::SmartFaceHandle& face) { return face.idx(); });
            result.add_elements(path, vector);
            break;
        }
        }

        return result;
    }

private:
    void update_selection(const SelectionList& selection, const Application::SelectionMode mode)
    {
        if (m_update_count == 1)
        {
            return;
        }

        if (selection.empty())
        {
            return;
        }

        const auto selection_data = selection[m_mesh.GetPath()];
        if (selection_data.empty())
        {
            return;
        }

        switch (mode)
        {
        case Application::SelectionMode::POINTS:
        case Application::SelectionMode::UV:
        {
            const auto selected_points = selection_data.get_point_index_intervals();
            m_half_edge.vertices().for_each([&](const OpenMesh::SmartVertexHandle& vertex) {
                const auto selected = selected_points.contains(vertex.idx());
                m_half_edge.status(vertex).set_selected(selected);
            });
            break;
        }
        case Application::SelectionMode::EDGES:
        {
            const auto selected_edges = selection_data.get_edge_index_intervals();
            m_half_edge.edges().for_each([&](const OpenMesh::SmartEdgeHandle& edge) {
                auto selected = false;
                for (const auto edge_index : m_edge_indices[edge])
                {
                    if (selected_edges.contains(edge_index))
                    {
                        selected = true;
                        break;
                    }
                }
                m_half_edge.status(edge).set_selected(selected);
            });
            break;
        }
        case Application::SelectionMode::FACES:
        {
            const auto selected_faces = selection_data.get_element_index_intervals();
            m_half_edge.faces().for_each([&](const OpenMesh::SmartFaceHandle& face) {
                const auto selected = selected_faces.contains(face.idx());
                m_half_edge.status(face).set_selected(selected);
            });
            break;
        }
        }
    }

    OpenMesh::SmartHalfedgeHandle get_half_edge(const GfVec2i& indices)
    {
        const auto v1 = m_half_edge.vertex_handle(indices[0]);
        const auto v2 = m_half_edge.vertex_handle(indices[1]);
        return m_half_edge.find_halfedge(v1, v2);
    }

    void create_selection_changed_callback()
    {
        if (!m_selection_changed)
        {
            m_selection_changed =
                Application::instance().register_event_callback(Application::EventType::SELECTION_CHANGED, [&]() { ++m_update_count; });
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // HalfEdgeImpl::Type
    //////////////////////////////////////////////////////////////////////////

    enum Type
    {
        Grow = true,
        Decrease = false,
    };

    //////////////////////////////////////////////////////////////////////////
    // HalfEdgeImpl::SelectionTable
    //////////////////////////////////////////////////////////////////////////

    template <Application::SelectionMode mode, Type type>
    friend struct SelectionTable;

    template <Application::SelectionMode mode, Type type>
    struct SelectionTable;

    template <Type type>
    struct SelectionTable<Application::SelectionMode::POINTS, type>
    {
        constexpr static const auto who_filtered = &OMHalfEdge::faces;
        static bool how_filtered(HalfEdgeImpl* self, const OpenMesh::SmartFaceHandle& face)
        {
            return face.vertices().any_of(OpenMesh::Predicates::Selected()) && face.vertices().any_of(!OpenMesh::Predicates::Selected());
        }

        constexpr static const auto who_add = &OpenMesh::SmartFaceHandle::vertices;
        static void how_add(HalfEdgeImpl* self, std::set<int>& to, const OpenMesh::SmartVertexHandle& vertex)
        {
            to.insert(vertex.idx());
            self->m_half_edge.status(vertex).set_selected(type);
        };

        static SelectionList to_selection_list(const PXR_NS::SdfPath& path, std::set<int>& indices)
        {
            SelectionList selection;
            selection.add_points(path, indices);
            return std::move(selection);
        }
    };

    template <Type type>
    struct SelectionTable<Application::SelectionMode::EDGES, type>
    {
        constexpr static const auto who_filtered = &OMHalfEdge::vertices;
        static bool how_filtered(HalfEdgeImpl* self, const OpenMesh::SmartVertexHandle& vertex)
        {
            if (self->m_update_count == 0)
            {
                // For the first Grow Selection for edges,
                // the selected edges need to be reselected because not all half-edges of these edges may be selected.
                const auto any_selected = vertex.edges().any_of(OpenMesh::Predicates::Selected());
                return any_selected || (any_selected && vertex.edges().any_of(!OpenMesh::Predicates::Selected()));
            }
            else
            {
                return vertex.edges().any_of(OpenMesh::Predicates::Selected()) && vertex.edges().any_of(!OpenMesh::Predicates::Selected());
            }
        }

        constexpr static const auto who_add = &OpenMesh::SmartVertexHandle::edges;
        static void how_add(HalfEdgeImpl* self, std::set<int>& to, const OpenMesh::SmartEdgeHandle& edge)
        {
            const auto index = self->m_edge_indices[edge];
            to.insert(index.begin(), index.end());
            self->m_half_edge.status(edge).set_selected(type);
        };

        static SelectionList to_selection_list(const PXR_NS::SdfPath& path, std::set<int>& indices)
        {
            SelectionList selection;
            selection.add_edges(path, indices);
            return std::move(selection);
        }
    };

    template <Type type>
    struct SelectionTable<Application::SelectionMode::FACES, type>
    {
        constexpr static const auto who_filtered = &OMHalfEdge::vertices;
        static bool how_filtered(HalfEdgeImpl* self, const OpenMesh::SmartVertexHandle& vertex)
        {
            return vertex.faces().any_of(OpenMesh::Predicates::Selected()) && vertex.faces().any_of(!OpenMesh::Predicates::Selected());
        }

        constexpr static const auto who_add = &OpenMesh::SmartVertexHandle::faces;
        static void how_add(HalfEdgeImpl* self, std::set<int>& to, const OpenMesh::SmartFaceHandle& face)
        {
            to.insert(face.idx());
            self->m_half_edge.status(face).set_selected(type);
        };

        static SelectionList to_selection_list(const PXR_NS::SdfPath& path, std::set<int>& indices)
        {
            SelectionList selection;
            selection.add_elements(path, indices);
            return std::move(selection);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // HalfEdgeImpl::select
    //////////////////////////////////////////////////////////////////////////

    template <Application::SelectionMode mode, HalfEdgeImpl::Type type>
    SelectionList select(const SelectionList& current)
    {
        using Table = SelectionTable<mode, type>;

        create_selection_changed_callback();
        update_selection(current, mode);
        const auto how_filtered = std::bind(Table::how_filtered, this, std::placeholders::_1);
        auto filtered = (m_half_edge.*Table::who_filtered)().filtered(how_filtered);
        std::set<int> add;
        const auto filtered_set = filtered.to_set();
        const auto how_add = std::bind(Table::how_add, this, std::ref(add), std::placeholders::_1);
        for (const auto element : filtered_set)
        {
            (element.*Table::who_add)().for_each(how_add);
        }
        m_update_count = 0;
        return Table::to_selection_list(m_mesh.GetPath(), add);
    }

private:
    OMHalfEdge m_half_edge;

    OpenMesh::EProp<std::vector<int>> m_edge_indices;

    // OpenMesh::EProp<int> m_edge_topology_id;
    OpenMesh::FProp<int> m_face_topology_id;
    OpenMesh::VProp<int> m_vertex_topology_id;

    PXR_NS::UsdGeomMesh m_mesh;

    Application::CallbackHandle m_selection_changed;
    int m_update_count = 0;
};

//////////////////////////////////////////////////////////////////////////
// HalfEdge
//////////////////////////////////////////////////////////////////////////

HalfEdge::HalfEdge() = default;

HalfEdge::~HalfEdge() = default;

/* static */
HalfEdgePtr HalfEdge::from_mesh(const PXR_NS::UsdGeomMesh& mesh, PXR_NS::UsdTimeCode time)
{
    HalfEdgePtr result = std::make_shared<HalfEdge>();
    result->m_impl = HalfEdgeImpl::from_mesh(mesh, time);
    return result->m_impl ? result : nullptr;
}

SelectionList HalfEdge::edge_loop_selection(const PXR_NS::GfVec2i& begin)
{
    return m_impl ? m_impl->edge_loop_selection(begin) : SelectionList();
}

SelectionList HalfEdge::grow_selection(const SelectionList& current)
{
    return m_impl ? m_impl->grow_selection(current) : SelectionList();
}

SelectionList HalfEdge::decrease_selection(const SelectionList& current)
{
    return m_impl ? m_impl->decrease_selection(current) : SelectionList();
}

SelectionList HalfEdge::topology_selection(const SelectionList& current)
{
    return m_impl ? m_impl->topology_selection(current) : SelectionList();
}

//////////////////////////////////////////////////////////////////////////
// HalfEdgeCache
//////////////////////////////////////////////////////////////////////////

static bool animated_topology(const PXR_NS::UsdPrim& prim)
{
    const auto mesh = UsdGeomMesh(prim);
    if (!mesh)
    {
        return false;
    }

    return mesh.GetFaceVertexCountsAttr().GetNumTimeSamples() > 1 || mesh.GetFaceVertexIndicesAttr().GetNumTimeSamples() > 1 ||
           mesh.GetHoleIndicesAttr().GetNumTimeSamples() > 1;
}

static PXR_NS::UsdTimeCode correct_time(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time)
{
    return animated_topology(prim) ? time : PXR_NS::UsdTimeCode::Default();
}

HalfEdgeCache::HalfEdgeCache() {}

HalfEdgeCache::~HalfEdgeCache() {}

HalfEdgePtr HalfEdgeCache::get_half_edge(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time /* = PXR_NS::UsdTimeCode::Default() */)
{
    update(prim, time);
    return m_cache[prim][correct_time(prim, time)];
}

bool HalfEdgeCache::contains(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time /* = PXR_NS::UsdTimeCode::Default() */)
{
    auto mesh_samples_find = TfMapLookupPtr(m_cache, prim);
    if (!mesh_samples_find)
    {
        return false;
    }

    const auto corrected_time = correct_time(prim, time);

    auto half_edge_find = mesh_samples_find->find(corrected_time);
    if (half_edge_find == mesh_samples_find->end())
    {
        return false;
    }

    return true;
}

void HalfEdgeCache::clear_at_time(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time)
{
    auto iter = m_cache.find(prim);
    if (iter == m_cache.end())
    {
        return;
    }

    iter->second.erase(time.GetValue());
    if (iter->second.empty())
    {
        m_cache.erase(iter);
    }
}

void HalfEdgeCache::clear_timesamples(const PXR_NS::UsdPrim& prim)
{
    m_cache.erase(prim);
}

void HalfEdgeCache::clear()
{
    m_cache.clear();
}

void HalfEdgeCache::update(const PXR_NS::UsdPrim& prim, PXR_NS::UsdTimeCode time)
{
    auto mesh_samples_find = TfMapLookupPtr(m_cache, prim);
    if (!mesh_samples_find)
    {
        mesh_samples_find = &(m_cache.insert({ prim, MeshSamples() }).first->second);
    }

    const auto corrected_time = correct_time(prim, time);

    auto half_edge_find = mesh_samples_find->find(corrected_time);
    if (half_edge_find != mesh_samples_find->end() && half_edge_find->second)
    {
        return;
    }

    m_cache[prim][corrected_time] = HalfEdge::from_mesh(UsdGeomMesh(prim), corrected_time);
}

OPENDCC_NAMESPACE_CLOSE
