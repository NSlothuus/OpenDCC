// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/xformCache.h"

#include <embree3/rtcore.h>
#include "opendcc/app/core/mesh_bvh.h"
#include "opendcc/base/logging/logger.h"

OPENDCC_NAMESPACE_OPEN

#if RTC_VERSION < 31202
RTC_NAMESPACE_OPEN;
#else
RTC_NAMESPACE_USE;
#endif

PXR_NAMESPACE_USING_DIRECTIVE

static const std::string g_embree_log_channel_name = "Embree";
static const std::string g_meshBVH_log_channel_name = "MeshBVH";

struct MeshBvh::MeshBvhImpl
{

    struct PointsInRadiusQueryResult
    {
        const MeshBvhImpl* self = nullptr;
        int target_geom_id = -1;
        std::unordered_set<int> unique_points;
    };

    static bool point_in_radius_query_fn(RTCPointQueryFunctionArguments* args);

    ~MeshBvhImpl();
    RTCDevice device = nullptr;
    RTCScene scene = nullptr;
    RTCBuffer points_buffer = nullptr;
    RTCGeometry geom;
    std::vector<GfVec3f> points_data;
    std::vector<int> triangle_indices;
    size_t num_triangles = 0;
    UsdGeomMesh usd_mesh;
    bool load_geometry(const UsdPrim& prim);
    bool update_geometry();
    bool inite_scene();
    bool cast_ray(GfVec3f origin, GfVec3f dir, GfVec3f& hit_point, GfVec3f& hit_normal) const;
    std::vector<int> get_points_in_radius(const PXR_NS::GfVec3f& point, float radius);
};

MeshBvh::MeshBvhImpl::~MeshBvhImpl()
{
    if (scene)
        rtcReleaseScene(scene);
    if (device)
        rtcReleaseDevice(device);
}

bool MeshBvh::MeshBvhImpl::point_in_radius_query_fn(RTCPointQueryFunctionArguments* args)
{
    auto result = reinterpret_cast<PointsInRadiusQueryResult*>(args->userPtr);
    if (result->target_geom_id != -1 && result->target_geom_id != args->geomID)
        return false;

    const auto query_point = GfVec3f(args->query->x, args->query->y, args->query->z);

    const auto triangle_idx = args->primID;
    const auto radius_sq = args->query->radius * args->query->radius;

    for (int i = triangle_idx * 3; i < triangle_idx * 3 + 3; ++i)
    {
        const auto point_idx = result->self->triangle_indices[i];
        const auto point = result->self->points_data[point_idx];
        const auto dist_sq = (query_point - point).GetLengthSq();
        if (dist_sq < radius_sq)
            result->unique_points.insert(point_idx);
    }

    return false;
}

bool MeshBvh::MeshBvhImpl::update_geometry()
{
    if (!device || !scene)
        return false;

    UsdTimeCode time = UsdTimeCode::Default();
    UsdGeomXformCache xform_cache(time);
    GfMatrix4d local2world = xform_cache.GetLocalToWorldTransform(usd_mesh.GetPrim());
    GfVec3f* vertices = (GfVec3f*)rtcGetGeometryBufferData(geom, RTC_BUFFER_TYPE_VERTEX, 0);

    VtVec3fArray points;
    if (usd_mesh.GetPointsAttr().Get(&points, time))
    {
        GfVec3f* P = points.data();
        for (size_t i = 0; i < points.size(); ++i)
            vertices[i] = GfVec3f(local2world.Transform(P[i]));
    }
    else
    {
        OPENDCC_ERROR("Fail to get points from {}", usd_mesh.GetPath().GetText());
        return false;
    }

    rtcUpdateGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0);
    rtcCommitGeometry(geom);
    rtcCommitScene(scene);
    return true;
}

bool MeshBvh::MeshBvhImpl::load_geometry(const UsdPrim& prim)
{
    if (!prim)
        return false;

    if (!prim.IsA<UsdGeomMesh>())
    {
        OPENDCC_ERROR("Input geometry in {} is not a UsdGeomMesh. It is {}", prim.GetPath().GetText(), prim.GetTypeName().GetText());
        return false;
    }
    UsdTimeCode time = UsdTimeCode::Default();
    usd_mesh = UsdGeomMesh(prim);
    UsdGeomXformCache xform_cache(time);
    GfMatrix4d local2world = xform_cache.GetLocalToWorldTransform(prim);

    VtVec3fArray points;
    if (usd_mesh.GetPointsAttr().Get(&points, time))
    {
        GfVec3f* P = points.data();
        points_data.resize(points.size());
        for (size_t i = 0; i < points.size(); ++i)
            points_data[i] = GfVec3f(local2world.Transform(P[i]));
    }
    else
    {
        OPENDCC_ERROR("Fail to get points from {}", prim.GetPath().GetText());
        return false;
    }

    VtIntArray polygons_count;
    bool ok = usd_mesh.GetFaceVertexCountsAttr().Get(&polygons_count, time);
    if (!ok)
    {
        OPENDCC_ERROR("Fail to get polygons_count from {}", prim.GetPath().GetText());
        return false;
    }

    VtIntArray polygons_indices;
    ok = usd_mesh.GetFaceVertexIndicesAttr().Get(&polygons_indices, time);
    if (!ok)
    {
        OPENDCC_ERROR("Fail to get polygons_indices from {}", prim.GetPath().GetText());
        return false;
    }
    auto polygons_indices_data = polygons_indices.cdata();
    // fan triangulation
    size_t polygons_start = 0;
    for (size_t polygon_ind = 0; polygon_ind < polygons_count.size(); ++polygon_ind)
    {
        if (polygons_count[polygon_ind] >= 3)
        {
            const uint32_t triangle_ind_0 = polygons_indices_data[polygons_start + 0];
            for (uint32_t triangle_ind_1_ind = 1; triangle_ind_1_ind < polygons_count[polygon_ind] - 1; ++triangle_ind_1_ind)
            {
                const uint32_t triangle_ind_1 = polygons_indices_data[polygons_start + triangle_ind_1_ind];
                const uint32_t triangle_ind_2 = polygons_indices_data[polygons_start + triangle_ind_1_ind + 1];
                triangle_indices.push_back(triangle_ind_0);
                triangle_indices.push_back(triangle_ind_1);
                triangle_indices.push_back(triangle_ind_2);
            }
        }
        polygons_start += polygons_count[polygon_ind];
    }
    return true;
}

bool MeshBvh::MeshBvhImpl::inite_scene()
{
    device = rtcNewDevice(nullptr);
    if (!device)
    {
        OPENDCC_ERROR("Failed to create embree rtc device.");
        return false;
    }

    rtcSetDeviceErrorFunction(device, [](void* user_ptr, RTCError error, const char* str) { OPENDCC_ERROR(str); }, nullptr);

    scene = rtcNewScene(device);
    rtcSetSceneFlags(scene, RTC_SCENE_FLAG_DYNAMIC);
    rtcSetSceneBuildQuality(scene, RTC_BUILD_QUALITY_LOW);

    geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);
    rtcSetGeometryBuildQuality(geom, RTC_BUILD_QUALITY_REFIT);
    points_data.reserve(points_data.size() + 1); // for 16-byte SSE https://www.embree.org/api.html#rtcreleasebuffer

    points_buffer = rtcNewSharedBuffer(device, points_data.data(), points_data.size() * sizeof(GfVec3f));
    rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, points_buffer, 0, 3 * sizeof(float), points_data.size());

    RTCBuffer indices_buffer = rtcNewSharedBuffer(device, triangle_indices.data(), triangle_indices.size() * sizeof(int32_t));
    rtcSetGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, indices_buffer, 0, 3 * sizeof(unsigned), triangle_indices.size() / 3);

    rtcCommitGeometry(geom);
    rtcAttachGeometry(scene, geom);
    rtcReleaseGeometry(geom);
    rtcCommitScene(scene);
    rtcReleaseBuffer(indices_buffer);
    return true;
}

bool MeshBvh::MeshBvhImpl::cast_ray(GfVec3f origin, GfVec3f dir, GfVec3f& hit_point, GfVec3f& hit_normal) const
{
    struct RTCIntersectContext context;
    rtcInitIntersectContext(&context);
    struct RTCRayHit rayhit;
    rayhit.ray.org_x = origin[0];
    rayhit.ray.org_y = origin[1];
    rayhit.ray.org_z = origin[2];
    rayhit.ray.dir_x = dir[0];
    rayhit.ray.dir_y = dir[1];
    rayhit.ray.dir_z = dir[2];
    rayhit.ray.tnear = 0;
    rayhit.ray.tfar = std::numeric_limits<float>::infinity();
    rayhit.ray.mask = -1;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

    rtcIntersect1(scene, &context, &rayhit);
    if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
    {
        hit_point = origin + rayhit.ray.tfar * dir;
        hit_normal = GfVec3f(rayhit.hit.Ng_x, rayhit.hit.Ng_y, rayhit.hit.Ng_z);
        hit_normal.Normalize();
        return true;
    }
    else
        return false;
}

std::vector<int> MeshBvh::MeshBvhImpl::get_points_in_radius(const PXR_NS::GfVec3f& point, float radius)
{
    PointsInRadiusQueryResult result;
    RTCPointQuery query;
    query.radius = radius;
    query.time = 0;
    query.x = point[0];
    query.y = point[1];
    query.z = point[2];

    result.self = this;

    RTCPointQueryContext context;
    rtcInitPointQueryContext(&context);
    rtcPointQuery(scene, &query, &context, point_in_radius_query_fn, &result);

    std::vector<int> points_indices;
    for (auto idx : result.unique_points)
        points_indices.push_back(idx);

    return points_indices;
}

MeshBvh::MeshBvh(const UsdPrim& prim)
{
    set_prim(prim);
}

MeshBvh::MeshBvh() {}

MeshBvh::~MeshBvh() {}

void MeshBvh::set_prim(const UsdPrim& prim)
{
    m_impl = std::make_unique<MeshBvhImpl>();
    bool ok = m_impl->load_geometry(prim);
    if (ok)
    {
        bool init_status = m_impl->inite_scene();
        if (!init_status)
            m_impl.reset();
    }
    else
    {
        m_impl.reset();
    }
}

bool MeshBvh::cast_ray(GfVec3f origin, GfVec3f dir, GfVec3f& hit_point, GfVec3f& hit_normal) const
{
    return m_impl->cast_ray(origin, dir, hit_point, hit_normal);
}

bool MeshBvh::is_valid() const
{
    return m_impl != nullptr;
}

bool MeshBvh::update_geometry()
{
    if (m_impl)
        return m_impl->update_geometry();
    else
        return false;
}

std::vector<int> MeshBvh::get_points_in_radius(const PXR_NS::GfVec3f& point, const PXR_NS::SdfPath& prim_path, float radius)
{
    if (m_impl)
        return m_impl->get_points_in_radius(point, radius);
    else
        return std::vector<int>();
}
OPENDCC_NAMESPACE_CLOSE
