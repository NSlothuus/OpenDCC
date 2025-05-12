// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/rich_selection.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include <pxr/usd/usdGeom/pointBased.h>
#include <pxr/base/work/loops.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

RichSelection::RichSelection(WeightFn weight_fn, ColorFn color_fn /*= ColorFn()*/)
{
    m_data = std::make_shared<RichSelectionData>();
    m_data->m_weight_fn = weight_fn;
    m_data->m_color_fn = color_fn;
}

RichSelection::RichSelection()
{
    m_data = std::make_shared<RichSelectionData>();
}

const RichSelection::WeightMap& RichSelection::get_weights(const SdfPath& prim_path) const
{
    static const WeightMap empty;
    auto iter = m_data->m_per_prim_weights.find(prim_path);
    return iter == m_data->m_per_prim_weights.end() ? empty : iter->second;
}

RichSelection::iterator RichSelection::begin()
{
    return m_data->m_per_prim_weights.begin();
}

RichSelection::const_iterator RichSelection::begin() const
{
    return m_data->m_per_prim_weights.begin();
}

RichSelection::iterator RichSelection::end()
{
    return m_data->m_per_prim_weights.end();
}

RichSelection::const_iterator RichSelection::end() const
{
    return m_data->m_per_prim_weights.end();
}

void RichSelection::clear()
{
    m_data->m_selection_list.clear();
    m_data->m_per_prim_weights.clear();
}

void RichSelection::set_soft_selection(const SelectionList& selection)
{
    // detach
    if (!m_data.unique())
    {
        m_data = std::make_shared<RichSelectionData>(*m_data);
        m_data->m_callback_handle = Session::StageChangedCallbackHandle();
    }

    m_data->m_selection_list = selection;
    m_data->m_bvh = std::make_shared<PointCloudBVH>();
    if (m_data->m_callback_handle)
        Application::instance().get_session()->unregister_stage_changed_callback(Session::StageChangedEventType::CURRENT_STAGE_OBJECT_CHANGED,
                                                                                 m_data->m_callback_handle);

    const auto& target_paths = m_data->m_selection_list.get_selected_paths();
    auto path_set = std::unordered_set<SdfPath, SdfPath::Hash>(target_paths.begin(), target_paths.end());
    m_data->m_callback_handle = Application::instance().get_session()->register_stage_changed_callback(
        Session::StageChangedEventType::CURRENT_STAGE_OBJECT_CHANGED, [path_set, bvh = m_data->m_bvh](const UsdNotice::ObjectsChanged& notice) {
            auto stage = notice.GetStage();
            for (const auto& entry : notice.GetResyncedPaths())
            {
                if (path_set.find(entry) == path_set.end())
                    continue;

                auto prim = stage->GetPrimAtPath(entry);
                if (!prim)
                    bvh->remove_prim(entry);
            }
            for (const auto& entry : notice.GetChangedInfoOnlyPaths())
            {
                if (path_set.find(entry.GetPrimPath()) == path_set.end())
                    continue;

                auto prim = stage->GetPrimAtPath(entry);
                auto point_based = UsdGeomPointBased(prim);
                if (entry.GetNameToken() == TfToken("points"))
                    bvh->remove_prim(entry.GetPrimPath());
            }
        });

    update();
}

GfVec3f RichSelection::get_soft_selection_color(float weight) const
{
    return m_data->m_color_fn ? m_data->m_color_fn(weight) : GfVec3f(0);
}

bool RichSelection::has_color_data() const
{
    return m_data->m_color_fn != nullptr;
}

void RichSelection::update()
{
    auto stage = Application::instance().get_session()->get_current_stage();
    if (!stage)
        return;

    m_data->m_per_prim_weights.clear();
    auto& topology_cache =
        Application::instance().get_session()->get_stage_topology_cache(Application::instance().get_session()->get_current_stage_id());
    auto time = Application::instance().get_current_time();
    const auto falloff_radius = Application::instance().get_settings()->get("soft_selection.falloff_radius", 5.0f);
    auto cache = UsdGeomXformCache(time);
    for (const auto& entry : m_data->m_selection_list)
    {
        const auto& path = entry.first;
        const auto& sel_data = entry.second;
        auto point_based = UsdGeomPointBased(stage->GetPrimAtPath(path));
        if (!point_based)
            continue;

        VtArray<GfVec3f> points;
        if (!point_based.GetPointsAttr().Get(&points, time))
            continue;
        const auto world_transform = cache.GetLocalToWorldTransform(point_based.GetPrim());

        auto try_add_weight = [this, &weights = m_data->m_per_prim_weights[path], world_transform, data = points.cdata()](size_t cur_point_ind,
                                                                                                                          size_t sel_point_ind) {
            if (cur_point_ind == sel_point_ind)
            {
                weights[cur_point_ind] = m_data->m_weight_fn(0.0);
            }
            else
            {
                auto w = m_data->m_weight_fn(
                    (world_transform.Transform(data[sel_point_ind]) - world_transform.Transform(data[cur_point_ind])).GetLength());
                if (w > 0.0)
                {
                    weights[cur_point_ind] = w;
                }
            }
        };

        if (!m_data->m_bvh->has_prim(path))
        {
            std::unordered_set<SelectionList::IndexType> point_indices;
            if (!sel_data.get_edge_indices().empty() || !sel_data.get_element_indices().empty())
            {
                const auto topology = topology_cache.get_topology(point_based.GetPrim(), time);
                point_indices.reserve(sel_data.get_point_indices().size() + 2 * sel_data.get_edge_indices().size() +
                                      topology->mesh_topology.GetFaceVertexIndices().size());

                for (const auto edge_ind : sel_data.get_edge_indices())
                {
                    auto verts = topology->edge_map.get_vertices_by_edge_id(edge_ind);
                    if (!std::get<1>(verts))
                        continue;
                    point_indices.insert(std::get<0>(verts)[0]);
                    point_indices.insert(std::get<0>(verts)[1]);
                }
                const auto& face_counts = topology->mesh_topology.GetFaceVertexCounts();
                const auto& face_indices = topology->mesh_topology.GetFaceVertexIndices();
                for (const auto face_ind : sel_data.get_element_indices())
                {
                    const auto face_start = topology->face_starts[face_ind];
                    for (int i = 0; i < face_counts[face_ind]; i++)
                        point_indices.insert(face_indices[face_start + i]);
                }
            }
            else
            {
                point_indices.reserve(sel_data.get_point_indices().size());
            }
            point_indices.insert(sel_data.get_point_indices().begin(), sel_data.get_point_indices().end());
            if (point_indices.empty())
                continue;
            m_data->m_bvh->add_prim(path, world_transform, points, VtIntArray(point_indices.begin(), point_indices.end()));
        }

        m_data->m_bvh->set_prim_transform(path, world_transform);
        WorkParallelForN(points.size(),
                         [this, path, world_transform, try_add_weight, falloff_radius, data = points.cdata()](size_t begin, size_t end) {
                             for (size_t i = begin; i < end; ++i)
                             {
                                 const auto& point = world_transform.Transform(data[i]);
                                 const auto nearest_ind = m_data->m_bvh->get_nearest_point(GfVec3f(point), path, falloff_radius);
                                 if (nearest_ind != -1)
                                     try_add_weight(i, nearest_ind);
                             }
                         });
    }
}

RichSelection::RichSelectionData::~RichSelectionData()
{
    if (m_callback_handle)
        Application::instance().get_session()->unregister_stage_changed_callback(Session::StageChangedEventType::CURRENT_STAGE_OBJECT_CHANGED,
                                                                                 m_callback_handle);
}
OPENDCC_NAMESPACE_CLOSE
