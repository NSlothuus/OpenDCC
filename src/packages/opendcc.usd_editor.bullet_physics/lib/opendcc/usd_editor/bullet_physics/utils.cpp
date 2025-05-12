// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/bullet_physics/utils.h"

// #include "GIMPACTUtils/btGImpactConvexDecompositionShape.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/tf/token.h"
#include "pxr/usd/usd/attribute.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdGeom/cube.h"
#include "pxr/usd/usdGeom/sphere.h"
#include "pxr/imaging/hd/meshUtil.h"
#include "btBulletDynamicsCommon.h"

#include "VHACD.h"
#include <iostream>
#include <vector>
#include "pxr/base/gf/transform.h"
#include "pxr/usd/usdGeom/xformCommonAPI.h"
#include "opendcc/app/core/undo/block.h"
#include "pxr/usd/usd/primRange.h"
#include "pxr/base/gf/vec3f.h"

PXR_NAMESPACE_USING_DIRECTIVE;

OPENDCC_NAMESPACE_OPEN

namespace
{

    template <class TResult>
    TResult get_attr_value(const UsdPrim& prim, const TfToken& attr, UsdTimeCode time)
    {
        TResult result;
        prim.GetAttribute(attr).Get<TResult>(&result, time);
        return result;
    }

    VtVec3iArray compute_triangles_indices(const UsdGeomMesh& mesh, UsdTimeCode time_code)
    {
        VtVec3iArray indices;
        auto mesh_topology = HdMeshTopology(get_attr_value<TfToken>(mesh.GetPrim(), UsdGeomTokens->subdivisionScheme, time_code),
                                            get_attr_value<TfToken>(mesh.GetPrim(), UsdGeomTokens->orientation, time_code),
                                            get_attr_value<VtIntArray>(mesh.GetPrim(), UsdGeomTokens->faceVertexCounts, time_code),
                                            get_attr_value<VtIntArray>(mesh.GetPrim(), UsdGeomTokens->faceVertexIndices, time_code),
                                            get_attr_value<VtIntArray>(mesh.GetPrim(), UsdGeomTokens->holeIndices, time_code));

        HdMeshUtil mesh_utils(&mesh_topology, mesh.GetPath());
        VtIntArray primitive_params;
        mesh_utils.ComputeTriangleIndices(&indices, &primitive_params);
        return indices;
    }

    struct TriangleMeshData
    {
        VtVec3fArray points;
        VtVec3iArray indices;
        btIndexedMesh indexed_mesh;
        btTriangleIndexVertexArray triangle_indexed_mesh;

        TriangleMeshData(const UsdGeomMesh& mesh, UsdTimeCode time_code, bool& ok)
        {
            ok = false;
            ok = mesh.GetPointsAttr().Get(&points);
            if (!ok || points.size() == 0)
                return;

            indices = compute_triangles_indices(mesh, time_code);

            indexed_mesh.m_numTriangles = indices.size();
            indexed_mesh.m_triangleIndexBase = (const unsigned char*)indices.cdata();
            indexed_mesh.m_triangleIndexStride = 3 * sizeof(uint32_t);
            indexed_mesh.m_numVertices = points.size();
            indexed_mesh.m_vertexBase = (const unsigned char*)points.cdata();
            indexed_mesh.m_vertexStride = sizeof(GfVec3f);
            triangle_indexed_mesh.addIndexedMesh(indexed_mesh);
            ok = true;
        }
    };

    //     class btGImpactConvexDecompositionShapeWithBuffers : public btGImpactConvexDecompositionShape
    //     {
    //     public:
    //         btGImpactConvexDecompositionShapeWithBuffers(
    //             btStridingMeshInterface* meshInterface,
    //             const btVector3& mesh_scale,
    //             btScalar margin = btScalar(0.01), bool children_has_transform = true) :
    //             btGImpactConvexDecompositionShape(meshInterface, mesh_scale, margin, children_has_transform)
    //         {}
    //         ~btGImpactConvexDecompositionShapeWithBuffers()
    //         {
    //             delete data;
    //         }
    //         TriangleMeshData* data = nullptr;
    //     };

    class btBvhTriangleMeshShapeWithBuffers : public btBvhTriangleMeshShape
    {
    public:
        btBvhTriangleMeshShapeWithBuffers(btStridingMeshInterface* meshInterface, bool useQuantizedAabbCompression, bool buildBvh = true)
            : btBvhTriangleMeshShape(meshInterface, useQuantizedAabbCompression, buildBvh)
        {
        }
        ~btBvhTriangleMeshShapeWithBuffers() { delete data; }
        TriangleMeshData* data = nullptr;
    };

    class btConvexHullShapeWithBuffers : public btConvexHullShape
    {
    public:
        btConvexHullShapeWithBuffers(const btScalar* points, int numPoints, int stride)
            : btConvexHullShape(points, numPoints, stride)
        {
        }
        ~btConvexHullShapeWithBuffers() {}
        std::shared_ptr<VtVec3fArray> points_buffer;
    };

    class btVHACDShapeWithBuffers : public btCompoundShape
    {
    public:
        using Buffer = std::shared_ptr<std::vector<float>>;
        btVHACDShapeWithBuffers() {}
        ~btVHACDShapeWithBuffers()
        {
            for (auto* shape : convex_shapes)
                delete shape;
        }
        std::vector<btConvexHullShape*> convex_shapes;
        std::vector<Buffer> points_buffers;
    };

    class CompoundShape : public btCompoundShape
    {
    public:
        using ChildrenMap = std::unordered_map<SdfPath, std::unique_ptr<btCollisionShape>, SdfPath::Hash>;

        CompoundShape(ChildrenMap&& children, const std::unordered_map<SdfPath, btTransform, SdfPath::Hash>& transforms)
        {
            m_children = std::move(children);
            for (auto& shape_it : m_children)
                addChildShape(transforms.at(shape_it.first), shape_it.second.get());
        }
        ~CompoundShape() {}
        void remove_chaild(const SdfPath& chaild_path)
        {
            if (m_children.find(chaild_path) != m_children.end())
            {
                this->removeChildShape(m_children.find(chaild_path)->second.get());
                m_children.erase(chaild_path);
            }
        }
        const ChildrenMap& children() const { return m_children; }

    private:
        ChildrenMap m_children;
    };

#ifdef VHACD_LOGGING
    class VHACDUpdate : public VHACD::IVHACD::IUserCallback
    {
    public:
        virtual ~VHACDUpdate() {}
        virtual void Update(const double overallProgress, const double stageProgress, const double operationProgress, const char* const stage,
                            const char* const operation)
        {
            OPENDCC_INFO("Stage " + std::string(stage) + "; operation " + std::string(operation) + "  " + std::to_string(operationProgress) + "  " +
                         std::to_string(stageProgress) + "  " + std::to_string(overallProgress));
        }
    };

    class VHACDLogger : public VHACD::IVHACD::IUserLogger
    {
    public:
        virtual ~VHACDLogger() {};
        virtual void Log(const char* const msg) { OPENDCC_WARN(msg); }
    };
#endif
}

void usd_transform_to_bullet(const GfTransform& from, btTransform& to)
{
    to.setIdentity();

    GfVec3d translate = from.GetTranslation();
    to.setOrigin(btVector3(translate[0], translate[1], translate[2]));

    GfQuatd q = from.GetRotation().GetQuat();
    q.Normalize();
    to.setRotation(btQuaternion(q.GetImaginary()[0], q.GetImaginary()[1], q.GetImaginary()[2], q.GetReal()));
}

std::unique_ptr<btCollisionShape> create_bvh_mesh_shape(const UsdGeomMesh& mesh, UsdTimeCode time_code)
{
    bool ok = false;
    auto data = new TriangleMeshData(mesh, time_code, ok);
    if (!ok)
        return nullptr;

    auto shape = std::make_unique<btBvhTriangleMeshShapeWithBuffers>(&data->triangle_indexed_mesh, true);
    shape->data = data;
    return shape;
}

void update_children(UsdPrim prim, btCollisionShape* shape, const std::unordered_set<SdfPath, SdfPath::Hash>& components)
{
    if (!prim || !shape || components.size() == 0)
        return;

    CompoundShape* compound_shape = dynamic_cast<CompoundShape*>(shape);
    if (!shape)
    {
        OPENDCC_ERROR("Coding error: shape is not compound");
        return;
    }
    std::unordered_set<SdfPath, SdfPath::Hash> paths_to_update;
    for (auto& component : components)
    {
        for (auto& it : compound_shape->children())
        {
            if (it.first.HasPrefix(component))
                paths_to_update.insert(it.first);
        }
    }
    if (paths_to_update.size() == 0)
        return;

    UsdGeomXformable xform(prim);
    UsdGeomXformCache xform_cache;
    auto base_transform_inv = xform_cache.GetLocalToWorldTransform(prim).GetInverse();

    for (auto& path : paths_to_update)
    {
        auto component_prim = prim.GetStage()->GetPrimAtPath(path);
        if (!component_prim.IsValid())
        {
            compound_shape->remove_chaild(path);
        }
        else
        {
            auto child_iter = compound_shape->children().find(path);
            if (child_iter == compound_shape->children().end())
                continue;
            auto childe_prim = prim.GetStage()->GetPrimAtPath(path);
            const auto transform_matrix = xform_cache.GetLocalToWorldTransform(childe_prim) * base_transform_inv;
            auto transform = GfTransform(transform_matrix);
            btTransform bt_transform;
            usd_transform_to_bullet(transform, bt_transform);
            compound_shape->removeChildShape(child_iter->second.get());
            compound_shape->addChildShape(bt_transform, child_iter->second.get());
        }
    }
}

std::unique_ptr<btCollisionShape> create_compound_shape(const UsdPrim& base_prim, const SdfPathVector& atomic_prim_path,
                                                        BulletPhysicsEngine::BodyType type,
                                                        BulletPhysicsEngine::MeshApproximationType mesh_approximation_type, UsdTimeCode time_code)
{
    std::unordered_map<SdfPath, std::unique_ptr<btCollisionShape>, SdfPath::Hash> atomic_shapes;
    std::unordered_map<SdfPath, btTransform, SdfPath::Hash> transforms;
    UsdGeomXformable xform(base_prim);
    UsdGeomXformCache xform_cache;
    auto base_transform_inv = xform_cache.GetLocalToWorldTransform(base_prim).GetInverse();

    for (auto& path : atomic_prim_path)
    {
        auto prim = base_prim.GetStage()->GetPrimAtPath(path);
        auto shape = create_collision_shape(prim, type, mesh_approximation_type, time_code);
        if (!shape)
            continue;
        const auto transform_matrix = xform_cache.GetLocalToWorldTransform(prim) * base_transform_inv;

        auto transform = GfTransform(transform_matrix);

        btTransform start_transform;
        usd_transform_to_bullet(transform, start_transform);

        GfVec3d scale = transform.GetScale();
        shape->setLocalScaling(btVector3(btScalar(scale[0]), btScalar(scale[1]), btScalar(scale[2])));
        atomic_shapes[path] = std::move(shape);
        transforms[path] = start_transform;
    }

    if (atomic_shapes.size() > 0)
        return std::make_unique<CompoundShape>(std::move(atomic_shapes), transforms);

    return nullptr;
}

std::unique_ptr<btCollisionShape> create_convex_hull_shape(const UsdGeomMesh& mesh, UsdTimeCode time_code)
{
    VtVec3fArray points;
    bool ok = mesh.GetPointsAttr().Get(&points);
    if (!ok)
        return nullptr;
    auto points_buffer = std::make_shared<VtVec3fArray>(points);
    auto shape = std::make_unique<btConvexHullShapeWithBuffers>((float*)points.data(), points.size(), sizeof(GfVec3f));
    shape->optimizeConvexHull();
    shape->points_buffer = points_buffer;
    return shape;
}

std::unique_ptr<btCollisionShape> create_box_shape(const UsdGeomMesh& mesh, UsdTimeCode time_code)
{
    VtVec3fArray extent;
    bool ok = mesh.GetExtentAttr().Get(&extent);
    if (!ok && extent.size() == 2)
        return nullptr;

    auto size = extent[1] - extent[0];
    auto center = (extent[1] + extent[0]) * 0.5;
    if (!GfIsClose(center, GfVec3f(0), 0.001f))
        OPENDCC_WARN("coding warning: Prim {} has extent with center != (0, 0, 0), BOX shape can be incorrect", mesh.GetPrim().GetPath().GetText());

    auto shape = std::make_unique<btBoxShape>(btVector3(btScalar(size[0] * 0.5), btScalar(size[1] * 0.5), btScalar(size[2] * 0.5)));
    return shape;
}

#ifdef UNUSED_OLD_CODE
// for some unknown reason did not work
std::unique_ptr<btCollisionShape> create_convex_decomposition_shape(const UsdGeomMesh& mesh, UsdTimeCode time_code)
{
    bool ok = false;
    auto data = new TriangleMeshData(mesh, time_code, ok);
    if (!ok)
        return nullptr;

    btVector3 scale(1.0f, 1.0f, 1.0f);

    auto shape = std::make_unique<btGImpactConvexDecompositionShapeWithBuffers>(&data->triangle_indexed_mesh, scale);
    shape->data = data;
    return shape;
}
#endif

std::unique_ptr<btCollisionShape> create_vhacd_shape(const UsdGeomMesh& mesh, UsdTimeCode time_code)
{
    using namespace VHACD;
    VtVec3fArray points;
    bool ok = mesh.GetPointsAttr().Get(&points);
    if (!ok || points.size() == 0)
        return nullptr;

    VtVec3iArray indices = compute_triangles_indices(mesh, time_code);
    if (indices.size() == 0)
        return nullptr;

    IVHACD* interfaceVHACD = CreateVHACD();
    IVHACD::Parameters paramsVHACD;
    paramsVHACD.m_resolution = 10000;
    paramsVHACD.m_depth = 10;

#ifdef VHACD_LOGGING
    VHACDLogger logger;
    VHACDUpdate updates;
    paramsVHACD.m_logger = &logger;
    paramsVHACD.m_callback = &updates;
#endif

    bool res = interfaceVHACD->Compute((const float*)points.cdata(), 3, (unsigned int)points.size(), (const int*)indices.data(), 3,
                                       (unsigned int)indices.size(), paramsVHACD);

    if (!res)
        return nullptr;

    unsigned int nConvexHulls = interfaceVHACD->GetNConvexHulls();
    if (nConvexHulls == 0)
        return nullptr;

    auto shape = std::make_unique<btVHACDShapeWithBuffers>();
    for (unsigned int p = 0; p < nConvexHulls; ++p)
    {
        IVHACD::ConvexHull ch;
        interfaceVHACD->GetConvexHull(p, ch);
        if (ch.m_nPoints == 0)
            continue;

        auto shape_points = std::make_shared<std::vector<float>>(ch.m_nPoints * 3);
        for (size_t i = 0; i < shape_points->size(); ++i)
            (*shape_points)[i] = ch.m_points[i];

        auto convex_shape = new btConvexHullShape(shape_points->data(), ch.m_nPoints, 3 * sizeof(float));
        convex_shape->optimizeConvexHull();

        shape->convex_shapes.push_back(convex_shape);
        shape->points_buffers.push_back(shape_points);
        shape->addChildShape(btTransform::getIdentity(), convex_shape);
    }

    interfaceVHACD->Cancel();
    interfaceVHACD->Release();

    if (shape->getNumChildShapes() == 0)
        return nullptr;

    return shape;
}

bool is_supported_type(const UsdPrim& prim)
{
    return prim && (prim.IsA<UsdGeomMesh>() || prim.IsA<UsdGeomCube>() || prim.IsA<UsdGeomSphere>());
}

SdfPathVector get_supported_prims_paths_recursively(const UsdStageRefPtr& stage, const SdfPathVector& paths)
{
    std::unordered_set<SdfPath, SdfPath::Hash> unique_paths;
    SdfPathVector result;
    for (auto path : paths)
    {
        UsdPrimRange range(stage->GetPrimAtPath(path.GetAbsoluteRootOrPrimPath()));
        for (auto sub_path = range.begin(); sub_path != range.end(); ++sub_path)
        {
            UsdPrim prim = stage->GetPrimAtPath(sub_path->GetPath());
            if (prim && is_supported_type(prim) && unique_paths.find(sub_path->GetPath()) == unique_paths.end())
            {
                result.push_back(sub_path->GetPath());
                unique_paths.insert(sub_path->GetPath());
            }
        }
    }
    return result;
}

std::unique_ptr<btCollisionShape> create_collision_shape(const UsdPrim& prim, BulletPhysicsEngine::BodyType type,
                                                         BulletPhysicsEngine::MeshApproximationType mesh_approximation_type, UsdTimeCode time_code)
{
    std::unique_ptr<btCollisionShape> shape;
    if (prim.IsA<UsdGeomSphere>())
    {
        double radius = 1;
        prim.GetAttribute(TfToken("radius")).Get(&radius);
        shape = std::make_unique<btSphereShape>(radius);
    }
    else if (prim.IsA<UsdGeomCube>())
    {
        double size = 1;
        prim.GetAttribute(TfToken("size")).Get(&size);
        shape = std::make_unique<btBoxShape>(btVector3(btScalar(size * 0.5), btScalar(size * 0.5), btScalar(size * 0.5)));
    }
    else if (prim.IsA<UsdGeomMesh>())
    {
        UsdGeomMesh mesh(prim);
        if (!mesh)
            return nullptr;

        if (type == BulletPhysicsEngine::BodyType::DYNAMIC)
        {
            if (mesh_approximation_type == BulletPhysicsEngine::MeshApproximationType::BOX)
                shape = create_box_shape(mesh, time_code);
            else if (mesh_approximation_type == BulletPhysicsEngine::MeshApproximationType::CONVEX_HULL)
                shape = create_convex_hull_shape(mesh, time_code);
            else if (mesh_approximation_type == BulletPhysicsEngine::MeshApproximationType::VHACD)
                shape = create_vhacd_shape(mesh, time_code);
            else
            {
                OPENDCC_INFO(BulletPhysicsEngine::s_extension_short_name, "coding error: trying creating dynamic shape with unsupported shape type");
                return nullptr;
            }
        }
        else if (type == BulletPhysicsEngine::BodyType::STATIC)
        {
            shape = create_bvh_mesh_shape(mesh, time_code);
        }
    }
    else if (prim.IsA<UsdGeomXformable>())
    {
        SdfPathVector atomic_prims = get_supported_prims_paths_recursively(prim.GetStage(), SdfPathVector(1, prim.GetPath()));
        shape = create_compound_shape(prim, atomic_prims, type, mesh_approximation_type, time_code);
    }

    return shape;
}

UsdTimeCode get_non_varying_time(const UsdAttribute& attr)
{
    if (!attr)
        return UsdTimeCode::Default();

    const auto num_time_samples = attr.GetNumTimeSamples();
    if (num_time_samples == 0)
    {
        return UsdTimeCode::Default();
    }
    else if (num_time_samples == 1)
    {
        std::vector<double> timesamples(1);
        attr.GetTimeSamples(&timesamples);
        return timesamples[0];
    }

    return UsdTimeCode::Default();
}

void decompose_to_common_api(const UsdGeomXformable& xform, const GfTransform& transform)
{
    static const GfVec3d zero_vec(0);
    static const GfVec3f one_vec(1);

    auto get_time = [&xform](const TfToken& attr_name) {
        return get_non_varying_time(xform.GetPrim().GetAttribute(attr_name));
    };

    auto xform_api = UsdGeomXformCommonAPI(xform);
    if (!GfIsClose(transform.GetTranslation(), zero_vec, 0.0001))
        xform_api.SetTranslate(transform.GetTranslation(), get_time(TfToken("xform:translate")));

    const auto euler_angles = transform.GetRotation().Decompose(GfVec3d::ZAxis(), GfVec3d::YAxis(), GfVec3d::XAxis());
    if (!GfIsClose(euler_angles, zero_vec, 0.0001))
        xform_api.SetRotate(GfVec3f(euler_angles[2], euler_angles[1], euler_angles[0]), UsdGeomXformCommonAPI::RotationOrder::RotationOrderXYZ,
                            get_time(TfToken("xform:rotateXYZ")));
    if (!GfIsClose(transform.GetScale(), one_vec, 0.0001))
        xform_api.SetScale(GfVec3f(transform.GetScale()), get_time(TfToken("xform:scale")));
    if (!GfIsClose(transform.GetPivotPosition(), zero_vec, 0.0001))
        xform_api.SetPivot(GfVec3f(transform.GetPivotPosition()), get_time(TfToken("xform:translate:pivot")));
}

void reset_pivots(const SdfPathVector& paths)
{
    auto stage = Application::instance().get_session()->get_current_stage();
    commands::UsdEditsUndoBlock undo_block;
    SdfChangeBlock change_block;
    auto cache = UsdGeomXformCache(Application::instance().get_current_time());
    std::vector<std::function<void()>> deferred_edits;
    for (const auto& path : paths)
    {
        auto prim = stage->GetPrimAtPath(path);
        if (!prim || cache.TransformMightBeTimeVarying(prim))
            continue;
        auto xform = UsdGeomXformable(stage->GetPrimAtPath(path));
        if (!xform)
            continue;
        bool dummy;
        auto local_mat = cache.GetLocalTransformation(prim, &dummy);
        GfTransform transform(local_mat);
        xform.ClearXformOpOrder();
        deferred_edits.emplace_back([xform, transform] { decompose_to_common_api(xform, transform); });
        prim.RemoveProperty(TfToken("xformOp:translate:pivot"));
    }

    if (!deferred_edits.empty())
    {
        SdfChangeBlock block;
        for (const auto& edit : deferred_edits)
            edit();
    }
}

OPENDCC_NAMESPACE_CLOSE
