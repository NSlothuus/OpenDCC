// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/core/point_cloud_bvh.h"
#include <embree3/rtcore.h>
#include <pxr/base/gf/matrix4f.h>
#include "opendcc/base/logging/logger.h"
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

#if RTC_VERSION < 31202
RTC_NAMESPACE_OPEN;
#else
RTC_NAMESPACE_USE;
#endif

PXR_NAMESPACE_USING_DIRECTIVE

static const std::string g_embree_log_channel_name = "Embree";

struct PointCloudBVH::PointCloudData
{
    struct CloudData
    {
        GfMatrix4d world_transform;
        RTCGeometry geom;
        VtVec3fArray points;
        VtIntArray indices;
    };
    RTCDevice device = nullptr;
    RTCScene scene = nullptr;
    std::unordered_map<SdfPath, unsigned, SdfPath::Hash> path_to_geom_id;
    std::unordered_map<unsigned, CloudData> cloud_data;

    struct PointQueryResult
    {
        const PointCloudData* data = nullptr;
        int target_geom_id = -1;
        int nearest_point_ind = -1;
    };

    struct PointsInRadiusQueryResult
    {
        const PointCloudData* data = nullptr;
        int target_geom_id = -1;
        std::vector<int> points_indices;
    };

    static bool point_query_fn(RTCPointQueryFunctionArguments* args)
    {
        auto result = reinterpret_cast<PointQueryResult*>(args->userPtr);
        if (result->target_geom_id != -1 && result->target_geom_id != args->geomID)
            return false;

        const auto& cloud_data = result->data->cloud_data.at(args->geomID);

        const auto point_ind = cloud_data.indices.empty() ? args->primID : cloud_data.indices[args->primID];
        const auto& world_transform = cloud_data.world_transform;
        const auto& point = world_transform.Transform(cloud_data.points[point_ind]);
        const auto query_point = GfVec3f(args->query->x, args->query->y, args->query->z);
        const auto dist = (query_point - point).GetLength();
        if (dist <= args->query->radius)
        {
            args->query->radius = dist;
            result->nearest_point_ind = point_ind;
            return true;
        }
        return false;
    }

    static bool point_in_radius_query_fn(RTCPointQueryFunctionArguments* args)
    {
        auto result = reinterpret_cast<PointsInRadiusQueryResult*>(args->userPtr);
        if (result->target_geom_id != -1 && result->target_geom_id != args->geomID)
            return false;

        const auto& cloud_data = result->data->cloud_data.at(args->geomID);

        const auto point_ind = cloud_data.indices.empty() ? args->primID : cloud_data.indices[args->primID];
        const auto& world_transform = cloud_data.world_transform;
        const auto& point = world_transform.Transform(cloud_data.points[point_ind]);
        const auto query_point = GfVec3f(args->query->x, args->query->y, args->query->z);
        const auto dist_sq = (query_point - point).GetLengthSq();
        if (dist_sq < args->query->radius * args->query->radius)
            result->points_indices.push_back(point_ind);
        return false;
    }

    static void bounds_fn(const RTCBoundsFunctionArguments* args)
    {
        const auto data = (const CloudData*)args->geometryUserPtr;
        const auto point_ind = data->indices.empty() ? args->primID : data->indices[args->primID];
        const auto& p = data->world_transform.Transform(data->points[point_ind]);
        RTCBounds bounds;
        bounds.lower_x = bounds.upper_x = p[0];
        bounds.lower_y = bounds.upper_y = p[1];
        bounds.lower_z = bounds.upper_z = p[2];
        *args->bounds_o = bounds;
    }

    PointCloudData()
    {
        device = rtcNewDevice(nullptr);
        if (!device)
        {
            OPENDCC_ERROR("Failed to create embree rtc device.");
            return;
        }

        rtcSetDeviceErrorFunction(device, [](void* user_ptr, RTCError error, const char* str) { OPENDCC_ERROR(str); }, nullptr);
        scene = rtcNewScene(device);
    }

    void add_prim(const SdfPath& prim_path, const GfMatrix4d& world, const VtVec3fArray& points, const VtIntArray& indices = VtIntArray())
    {
        auto geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_USER);
        rtcSetGeometryUserPrimitiveCount(geom, indices.empty() ? points.size() : indices.size());
        rtcSetGeometryBoundsFunction(geom, &bounds_fn, nullptr);
        rtcCommitGeometry(geom);
        const auto geom_id = rtcAttachGeometry(scene, geom);
        path_to_geom_id[prim_path] = geom_id;
        auto& entry = cloud_data[geom_id];
        entry.world_transform = world;
        entry.geom = geom;
        entry.points = points;
        entry.indices = indices;
        rtcSetGeometryUserData(geom, &entry);
        rtcReleaseGeometry(geom);
        rtcCommitScene(scene);
    }

    void remove_prim(const SdfPath& prim_path)
    {
        auto iter = path_to_geom_id.find(prim_path);
        if (iter == path_to_geom_id.end())
            return;

        const auto geom_id = iter->second;
        rtcDetachGeometry(scene, geom_id);
        rtcCommitScene(scene);
        cloud_data.erase(geom_id);
        path_to_geom_id.erase(iter);
    }

    void clear()
    {
        for (auto& it : path_to_geom_id)
        {
            const auto geom_id = it.second;
            rtcDetachGeometry(scene, geom_id);
        }
        rtcCommitScene(scene);
        cloud_data.clear();
        path_to_geom_id.clear();
    }

    int get_nearest_point(const GfVec3f& point, const SdfPath& prim_path, float max_radius)
    {
        RTCPointQuery query;
        query.radius = max_radius;
        query.time = 0;
        query.x = point[0];
        query.y = point[1];
        query.z = point[2];

        PointQueryResult result;
        result.data = this;
        result.nearest_point_ind = -1;
        if (!prim_path.IsEmpty())
        {
            auto iter = path_to_geom_id.find(prim_path);
            result.target_geom_id = iter == path_to_geom_id.end() ? -1 : iter->second;
        }
        else
        {
            result.target_geom_id = -1;
        }

        RTCPointQueryContext context;
        rtcInitPointQueryContext(&context);
        rtcPointQuery(scene, &query, &context, point_query_fn, &result);
        return result.nearest_point_ind;
    }

    std::vector<int> get_points_in_radius(const GfVec3f& point, const SdfPath& prim_path, float radius)
    {
        PointsInRadiusQueryResult result;
        RTCPointQuery query;
        query.radius = radius;
        query.time = 0;
        query.x = point[0];
        query.y = point[1];
        query.z = point[2];

        result.data = this;
        if (!prim_path.IsEmpty())
        {
            auto iter = path_to_geom_id.find(prim_path);
            result.target_geom_id = iter == path_to_geom_id.end() ? -1 : iter->second;
        }
        else
        {
            result.target_geom_id = -1;
        }

        RTCPointQueryContext context;
        rtcInitPointQueryContext(&context);
        rtcPointQuery(scene, &query, &context, point_in_radius_query_fn, &result);

        return result.points_indices;
    }

    ~PointCloudData()
    {
        if (scene)
            rtcReleaseScene(scene);
        if (device)
            rtcReleaseDevice(device);
    }
};

PointCloudBVH::PointCloudBVH()
{
    m_data = new PointCloudData;
}

PointCloudBVH::~PointCloudBVH()
{
    delete m_data;
}

int PointCloudBVH::get_nearest_point(const GfVec3f& point, const SdfPath& prim_path, float max_radius)
{
    return m_data->get_nearest_point(point, prim_path, max_radius);
}

std::vector<int> PointCloudBVH::get_points_in_radius(const GfVec3f& point, const SdfPath& prim_path /*= SdfPath()*/, float radius)
{
    return m_data->get_points_in_radius(point, prim_path, radius);
}

bool PointCloudBVH::has_prim(const SdfPath& prim_path) const
{
    return m_data->path_to_geom_id.find(prim_path) != m_data->path_to_geom_id.end();
}

void PointCloudBVH::add_prim(const SdfPath& prim_path, const GfMatrix4d& world, const VtVec3fArray& points,
                             const VtIntArray& indices /*= VtIntArray()*/)
{
    m_data->add_prim(prim_path, world, points, indices);
}

void PointCloudBVH::remove_prim(const SdfPath& prim_path)
{
    m_data->remove_prim(prim_path);
}

void PointCloudBVH::set_prim_transform(const SdfPath& prim_path, const GfMatrix4d& world_transform)
{
    auto iter = m_data->path_to_geom_id.find(prim_path);
    if (iter == m_data->path_to_geom_id.end())
        return;
    const auto geom_id = iter->second;
    if (GfIsClose(m_data->cloud_data[geom_id].world_transform, world_transform, 0.00001))
        return;

    m_data->cloud_data[geom_id].world_transform = world_transform;
    rtcCommitGeometry(m_data->cloud_data[geom_id].geom);
    rtcCommitScene(m_data->scene);
}

void PointCloudBVH::clear()
{
    m_data->clear();
}
OPENDCC_NAMESPACE_CLOSE
