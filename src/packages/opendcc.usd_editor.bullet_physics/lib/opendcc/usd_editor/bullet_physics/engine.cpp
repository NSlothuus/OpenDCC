// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/editContext.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>

#include "opendcc/app/core/undo/stack.h"
#include "opendcc/app/core/application.h"
#include "opendcc/app/core/session.h"
#include "opendcc/base/logging/logger.h"

#include <fstream>
#include <iostream>
#include "opendcc/usd_editor/bullet_physics/engine.h"

#include "btBulletDynamicsCommon.h"
#include "pxr/base/gf/quaternion.h"
#include "pxr/base/gf/rotation.h"
#include "opendcc/usd_editor/bullet_physics/debug_drawer.h"
#include "opendcc/usd_editor/bullet_physics/entry_point.h"
#include "opendcc/usd_editor/bullet_physics/utils.h"
#include "pxr/base/gf/transform.h"
#include "pxr/usd/usdGeom/metrics.h"
#include "pxr/imaging/hd/primGather.h"

PXR_NAMESPACE_USING_DIRECTIVE;

OPENDCC_NAMESPACE_OPEN

namespace
{
    static const float lever = 10.0f; // todo : make it adaptive to body size
    static const int num_pick_constraints_per_object = 6;
#ifdef UNUSED_OLD_CODE
    btQuaternion to_quaternion(const GfVec3d& euler_angles)
    {
        auto qd = (GfRotation(GfVec3d(1, 0, 0), euler_angles[0]) * GfRotation(GfVec3d(0, 1, 0), euler_angles[1]) *
                   GfRotation(GfVec3d(0, 0, 1), euler_angles[2]))
                      .GetQuat();
        auto i = qd.GetImaginary();
        auto q = GfQuatd(qd.GetReal(), i[0], i[1], i[2]);
        q.Normalize();
        btQuaternion bt_q(q.GetImaginary()[0], q.GetImaginary()[1], q.GetImaginary()[2], q.GetReal());
        return bt_q;
    }
#endif
}
const std::string BulletPhysicsEngine::s_extension_short_name = "Physics";

std::vector<BulletPhysicsEngine::BodyInfo> BulletPhysicsEngine::remove_children_from_paths_list(const std::vector<BodyInfo>& in)
{
    std::vector<BodyInfo> result;
    for (size_t i = 0; i < in.size(); ++i)
    {
        bool is_top = true;
        for (size_t j = 0; j < in.size(); ++j)
        {
            if (i != j && in[i].path.HasPrefix(in[j].path))
            {
                is_top = false;
                break;
            }
        }
        if (is_top)
            result.push_back(in[i]);
    }

    return result;
}

BulletPhysicsEngine::BulletPhysicsEngine(UsdStageRefPtr stage, double time)
{
    m_stage = stage;
    m_last_time = time;
    m_collision_configuration = std::make_unique<btDefaultCollisionConfiguration>();
    m_cillision_dispatcher = std::make_unique<btCollisionDispatcher>(m_collision_configuration.get());
    m_overlapping_pair_cache = std::make_unique<btDbvtBroadphase>();
    m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    m_dynamics_world = std::make_unique<btDiscreteDynamicsWorld>(m_cillision_dispatcher.get(), m_overlapping_pair_cache.get(), m_solver.get(),
                                                                 m_collision_configuration.get());

    m_dynamics_world->setGravity(btVector3(0, 0, 0));
    m_options.read_from_settings(s_extension_short_name);

    m_objects_changed_notice_key = TfNotice::Register(TfCreateWeakPtr(this), &BulletPhysicsEngine::on_objects_changed, m_stage);
    update_gravity_direction();
}

// need for boost python wrapping
BulletPhysicsEngine::BulletPhysicsEngine(const BulletPhysicsEngine&)
{
    OPENDCC_ERROR("Coding error: BulletPhysicsEngine copy constructor not implemented");
}

BulletPhysicsEngine::~BulletPhysicsEngine()
{
    TfNotice::Revoke(m_objects_changed_notice_key);
    // be sure of the correct deletion order
    m_dynamics_world.reset();
    m_solver.reset();
    m_overlapping_pair_cache.reset();
    m_cillision_dispatcher.reset();
    m_collision_configuration.reset();
}

void BulletPhysicsEngine::update_data_in_stage()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    SdfChangeBlock change_block;
    PXR_NS::UsdGeomXformCache xform_cache;
    for (auto& it : m_bodies)
    {
        if (it.second.type != BodyType::DYNAMIC)
            continue;

        auto& rigid_body = it.second.rigid_body;
        rigid_body->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
        btTransform bullet_transform;
        if (rigid_body && rigid_body->getMotionState())
        {
            rigid_body->getMotionState()->getWorldTransform(bullet_transform);
        }
        else
        {
            OPENDCC_WARN("coding warning: fail to get transform from {}", it.first.GetText());
            continue;
        }

        auto xform_api = UsdGeomXformCommonAPI(it.second.prim);
        if (!xform_api)
            continue;

        auto parent_to_world_inv = xform_cache.GetParentToWorldTransform(it.second.prim).GetInverse();
        auto world_translate = GfVec3d { float(bullet_transform.getOrigin().getX()), float(bullet_transform.getOrigin().getY()),
                                         float(bullet_transform.getOrigin().getZ()) };
        auto bullet_rotation = bullet_transform.getBasis();
        GfMatrix3d world_rotation_matrix(bullet_rotation[0][0], bullet_rotation[1][0], bullet_rotation[2][0], bullet_rotation[0][1],
                                         bullet_rotation[1][1], bullet_rotation[2][1], bullet_rotation[0][2], bullet_rotation[1][2],
                                         bullet_rotation[2][2]);

        auto local_transform_matrix = GfMatrix4d(world_rotation_matrix, world_translate) * parent_to_world_inv;
        GfTransform local_transform(local_transform_matrix);

        xform_api.SetTranslate(local_transform.GetTranslation());

        GfVec3d local_rotation = local_transform.GetRotation().Decompose(GfVec3d::ZAxis(), GfVec3d::YAxis(), GfVec3d::XAxis());
        xform_api.SetRotate(GfVec3f(local_rotation[2], local_rotation[1], local_rotation[0]));
    }
}

void BulletPhysicsEngine::step_simulation(bool add_gravity)
{
    if (m_bodies.size() == 0)
        return;
    m_miss_objects_changed = true;
    remove_pick_constraints();
    if (add_gravity)
        m_dynamics_world->setGravity(btVector3(m_gravity[0], m_gravity[1], m_gravity[2]));
    m_dynamics_world->stepSimulation(1.0, 100);
    if (add_gravity)
        m_dynamics_world->setGravity(btVector3(0, 0, 0));
    m_need_to_update_pick_constrins = true;
    update_data_in_stage();
    m_miss_objects_changed = false;
}

void BulletPhysicsEngine::add_pick_constraints(RigidBody& body)
{
    if (body.type != BodyType::DYNAMIC || body.pick_constraints.size() != 0)
        return;

    auto axis_X = btVector3 { lever, 0, 0 };
    auto axis_Y = btVector3 { 0, lever, 0 };
    auto axis_Z = btVector3 { 0, 0, lever };

    body.pick_constraints.resize(num_pick_constraints_per_object);

    body.pick_constraints[0] = std::make_unique<btPoint2PointConstraint>(*body.rigid_body, axis_X);
    body.pick_constraints[1] = std::make_unique<btPoint2PointConstraint>(*body.rigid_body, -axis_X);

    body.pick_constraints[2] = std::make_unique<btPoint2PointConstraint>(*body.rigid_body, axis_Y);
    body.pick_constraints[3] = std::make_unique<btPoint2PointConstraint>(*body.rigid_body, -axis_Y);

    body.pick_constraints[4] = std::make_unique<btPoint2PointConstraint>(*body.rigid_body, axis_Z);
    body.pick_constraints[5] = std::make_unique<btPoint2PointConstraint>(*body.rigid_body, -axis_Z);

    for (auto& it : body.pick_constraints)
    {
        it->m_setting.m_impulseClamp = m_options.pick_constraint_impulse_clamp;
        it->m_setting.m_tau = m_options.pick_constraint_tau;
        m_dynamics_world->addConstraint(it.get(), true);
    }
}

void BulletPhysicsEngine::create_pick_constraints(const SdfPathVector& paths)
{
    for (auto path : paths)
    {
        auto it = m_bodies.find(path);
        if (it == m_bodies.end())
            continue;

        auto& body = it->second;
        if (body.type != BodyType::DYNAMIC)
            continue;

        add_pick_constraints(body);
        m_picked_dyn_bodies.push_back(path);
    }
}

void BulletPhysicsEngine::remove_pick_constraints(RigidBody& body)
{
    for (auto& it : body.pick_constraints)
    {
        m_dynamics_world->removeConstraint(it.get());
        it.reset();
    }
    body.pick_constraints.clear();
}

void BulletPhysicsEngine::remove_pick_constraints()
{
    for (auto path : m_picked_dyn_bodies)
    {
        auto it = m_bodies.find(path);
        if (it != m_bodies.end())
            remove_pick_constraints(it->second);
    }
    if (m_dynamics_world->getNumConstraints() != 0)
        OPENDCC_ERROR("Coding error: dynamics_world->getNumConstraints() != 0");
    m_picked_dyn_bodies.clear();
}

void BulletPhysicsEngine::update_pick_constraints()
{
    auto selected_paths = Application::instance().get_selection().get_fully_selected_paths();
    remove_pick_constraints();
    create_pick_constraints(get_subtree_bodies_paths(selected_paths));
    m_need_to_update_pick_constrins = false;
}

void BulletPhysicsEngine::on_selection_changed()
{
    if (m_bodies.size() == 0)
        return;
    update_pick_constraints();
}

void BulletPhysicsEngine::deactivate()
{
    for (auto& body : m_bodies)
        m_deactivated_prims.emplace_back(body.second.path, body.second.type, body.second.mesh_approximation_type);

    remove_all();
    m_is_active = false;
    BulletPhysicsViewportUIExtension::update_gl();
}

void BulletPhysicsEngine::activate()
{
    m_is_active = true;
    add_objects(m_deactivated_prims);
    m_deactivated_prims.clear();
    BulletPhysicsViewportUIExtension::update_gl();
}

bool BulletPhysicsEngine::is_active() const
{
    return m_is_active;
}

void BulletPhysicsEngine::create_selected_prims_as_static_object()
{
    std::vector<BodyInfo> bodys_info;
    SdfPathVector paths = Application::instance().get_selection().get_fully_selected_paths();
    for (auto path : paths)
        bodys_info.emplace_back(path, BodyType::STATIC, MeshApproximationType::NONE);

    add_objects(bodys_info);
    BulletPhysicsViewportUIExtension::update_gl();
}

void BulletPhysicsEngine::create_selected_prims_as_dynamic_object(MeshApproximationType mesh_approximation_type)
{
    std::vector<BodyInfo> bodys_info;
    SdfPathVector paths = Application::instance().get_selection().get_fully_selected_paths();
    for (auto path : paths)
        bodys_info.emplace_back(path, BodyType::DYNAMIC, mesh_approximation_type);

    add_objects(bodys_info);
    BulletPhysicsViewportUIExtension::update_gl();
}

SdfPathVector BulletPhysicsEngine::get_subtree_bodies_paths(const SdfPathVector& interesting_paths, bool add_parents, bool add_children)
{
    std::unordered_set<SdfPath, SdfPath::Hash> unique_paths;
    SdfPathVector result;
    HdPrimGather gather;
    if (add_children)
    {
        for (auto path : interesting_paths)
        {
            SdfPathVector paths;
            gather.Subtree(m_bodies_sorted_paths.GetIds(), path, &paths);
            for (SdfPath& path : paths)
                unique_paths.insert(path);
        }
    }
    if (add_parents)
    {
        for (auto path : interesting_paths)
        {
            auto parent = parent_object(path);
            if (!parent.IsEmpty())
                unique_paths.insert(parent);
        }
    }

    for (auto& path : unique_paths)
        result.push_back(path);

    return result;
}

void BulletPhysicsEngine::remove_selected_prims_from_dym_scene()
{
    remove_objects(get_subtree_bodies_paths(Application::instance().get_selection().get_fully_selected_paths()));
    BulletPhysicsViewportUIExtension::update_gl();
}

void BulletPhysicsEngine::remove_all()
{
    remove_objects(get_subtree_bodies_paths(SdfPathVector(1, SdfPath::AbsoluteRootPath())));
    BulletPhysicsViewportUIExtension::update_gl();
}

void BulletPhysicsEngine::update_solver_options()
{
    m_options.read_from_settings(s_extension_short_name);
    update_gravity_direction();
    for (auto& it : m_bodies)
    {
        auto& body = it.second;
        body.rigid_body->setFriction(m_options.friction);
        body.rigid_body->setRestitution(m_options.restitution);
        body.rigid_body->setDamping(m_options.linear_damping, m_options.angular_damping);

        for (auto& it : body.pick_constraints)
        {
            it->m_setting.m_impulseClamp = m_options.pick_constraint_impulse_clamp;
            it->m_setting.m_tau = m_options.pick_constraint_tau;
        }
    }
}

void BulletPhysicsEngine::print_state() const
{
    OPENDCC_INFO("---------------- BulletPhysicsEngine --------------------");

    OPENDCC_INFO("DYNAMIC");
    for (auto& it : m_bodies)
        if (it.second.type == BodyType::DYNAMIC)
            OPENDCC_INFO("    {}", it.first.GetText());
    OPENDCC_INFO("");

    OPENDCC_INFO("STATIC");
    for (auto& it : m_bodies)
        if (it.second.type == BodyType::STATIC)
            OPENDCC_INFO("    {}", it.first.GetText());
    OPENDCC_INFO("");

    OPENDCC_INFO("PICKED");
    for (auto& it : m_picked_dyn_bodies)
        OPENDCC_INFO("    {}", it.GetText());
    OPENDCC_INFO("");

    OPENDCC_INFO(" objects usd: {} bullet: {}", std::to_string(m_bodies.size()).c_str(),
                 std::to_string(m_dynamics_world->getNumCollisionObjects()).c_str());
    OPENDCC_INFO(" constraints usd: {} bullet: {}", m_picked_dyn_bodies.size() * 6, m_dynamics_world->getNumConstraints());

    OPENDCC_INFO("------------------------------------");
}

void BulletPhysicsEngine::set_debug_drawer(btIDebugDraw* debugDrawer)
{
    m_dynamics_world->setDebugDrawer(debugDrawer);
}

btIDebugDraw* BulletPhysicsEngine::get_debug_drawer() const
{
    return m_dynamics_world->getDebugDrawer();
}

void BulletPhysicsEngine::draw_world()
{
    m_dynamics_world->debugDrawWorld();
}

void BulletPhysicsEngine::add_objects(const std::vector<BodyInfo>& bodys_info)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    UsdGeomXformCache xform_cache;
    auto bodys_info_cleaned = remove_children_from_paths_list(bodys_info);

    SdfPathVector paths;
    for (auto& info : bodys_info_cleaned)
        paths.push_back(info.path);

    remove_objects(get_subtree_bodies_paths(paths, true, true));

    for (auto& info : bodys_info_cleaned)
    {
        auto it = m_bodies.find(info.path);
        if (it != m_bodies.end())
        {
            if (it->second.type == info.type)
            {
                continue;
            }
            else
            {
                OPENDCC_WARN("coding error: bullet object {} already created", info.path.GetText());
                continue;
            }
        }

        auto prim = m_stage->GetPrimAtPath(info.path);
        if (!prim)
            continue;

        RigidBody body;
        body.type = info.type;
        body.prim = prim;
        body.path = info.path;
        body.mesh_approximation_type = info.mesh_approximation_type;
        body.shape = create_collision_shape(prim, info.type, info.mesh_approximation_type, Application::instance().get_current_time());
        if (!body.shape)
            continue;

        const auto transform_matrix = xform_cache.GetLocalToWorldTransform(prim);
        auto transform = GfTransform(transform_matrix);
        btTransform start_transform;
        usd_transform_to_bullet(transform, start_transform);

        GfVec3d scale = transform.GetScale();
        body.shape->setLocalScaling(btVector3(btScalar(scale[0]), btScalar(scale[1]), btScalar(scale[2])));

#ifdef UNUSED_OLD_VERSION
        if (auto attr = prim.GetAttribute(TfToken("xformOp:translate")))
        {
            GfVec3d translate;
            bool ok = attr.Get(&translate);
            if (ok)
                start_transform.setOrigin(btVector3(translate[0], translate[1], translate[2]));
        }

        if (auto attr = prim.GetAttribute(TfToken("xformOp:rotateXYZ")))
        {
            GfVec3f rotateXYZ;
            bool ok = attr.Get(&rotateXYZ);
            if (ok)
                start_transform.setRotation(to_quaternion(rotateXYZ));
        }

        if (auto attr = prim.GetAttribute(TfToken("xformOp:scale")))
        {
            GfVec3f scale(1.0f, 1.0f, 1.0f);
            bool ok = attr.Get(&scale);
            if (ok)
                body.shape->setLocalScaling(btVector3(btScalar(scale[0]), btScalar(scale[1]), btScalar(scale[2])));
        }
#endif

        btDefaultMotionState* motion_state = new btDefaultMotionState(start_transform);

        btVector3 local_inertia(0, 0, 0);
        btScalar mass(0);

        if (info.type == BodyType::DYNAMIC)
        {
            mass = btScalar(1.0);
            body.shape->calculateLocalInertia(mass, local_inertia);
        }

        body.rigid_body = std::make_unique<btRigidBody>(mass, motion_state, body.shape.get(), local_inertia);

        body.rigid_body->setFriction(m_options.friction);
        body.rigid_body->setRestitution(m_options.restitution);
        if (info.type == BodyType::DYNAMIC)
        {
            body.rigid_body->setActivationState(DISABLE_DEACTIVATION);
            body.rigid_body->setDamping(m_options.linear_damping, m_options.angular_damping);
        }

        m_dynamics_world->addRigidBody(body.rigid_body.get());

        if (info.type == BodyType::DYNAMIC)
        {
            m_picked_dyn_bodies.push_back(info.path);
            add_pick_constraints(body);
        }
        m_bodies[info.path] = std::move(body);
        m_bodies_sorted_paths.Insert(info.path);
    }
}

void BulletPhysicsEngine::remove_objects(const SdfPathVector& paths)
{
    SdfPathVector paths_to_delete;
    for (auto& path : paths)
    {
        auto it = m_bodies.find(path);
        if (it == m_bodies.end())
            continue;
        auto& body = it->second;
        remove_pick_constraints(body);
        m_dynamics_world->removeCollisionObject(body.rigid_body.get());
        paths_to_delete.push_back(path);
    }
    for (auto& path : paths_to_delete)
    {
        m_bodies_sorted_paths.Remove(path);
        m_bodies.erase(path);
    }
    if (paths_to_delete.size() > 0)
        update_pick_constraints();
}

void BulletPhysicsEngine::update_transforms_in_bullet(RigidBody& body, bool& bullet_scene_updated)
{
    UsdGeomXformCache xform_cache;
    if (body.type == BodyType::DYNAMIC)
    {
        const auto transform_matrix = xform_cache.GetLocalToWorldTransform(body.prim);
        GfTransform transform(transform_matrix);
        auto translate = transform.GetTranslation();
        auto rotation = transform.GetRotation();
        GfVec3d axis_X = rotation.TransformDir(GfVec3f { lever, 0, 0 });
        GfVec3d axis_Y = rotation.TransformDir(GfVec3f { 0, lever, 0 });
        GfVec3d axis_Z = rotation.TransformDir(GfVec3f { 0, 0, lever });

        if (body.pick_constraints.size() == num_pick_constraints_per_object)
        {
            body.pick_constraints[0]->setPivotB(btVector3(translate[0] + axis_X[0], translate[1] + axis_X[1], translate[2] + axis_X[2]));
            body.pick_constraints[1]->setPivotB(btVector3(translate[0] - axis_X[0], translate[1] - axis_X[1], translate[2] - axis_X[2]));

            body.pick_constraints[2]->setPivotB(btVector3(translate[0] + axis_Y[0], translate[1] + axis_Y[1], translate[2] + axis_Y[2]));
            body.pick_constraints[3]->setPivotB(btVector3(translate[0] - axis_Y[0], translate[1] - axis_Y[1], translate[2] - axis_Y[2]));

            body.pick_constraints[4]->setPivotB(btVector3(translate[0] + axis_Z[0], translate[1] + axis_Z[1], translate[2] + axis_Z[2]));
            body.pick_constraints[5]->setPivotB(btVector3(translate[0] - axis_Z[0], translate[1] - axis_Z[1], translate[2] - axis_Z[2]));
        }

        GfVec3d scale = transform.GetScale();
        body.shape->setLocalScaling(btVector3(btScalar(scale[0]), btScalar(scale[1]), btScalar(scale[2])));
        bullet_scene_updated = true;

#ifdef UNUSED_OLD_VARIANT
        auto attr_name = path.GetNameToken();
        if (attr_name == TfToken("xformOp:translate") || attr_name == TfToken("xformOp:rotateXYZ"))
        {
            GfVec3d translate(0, 0, 0);
            bool ok = body.prim.GetAttribute(TfToken("xformOp:translate")).Get(&translate);

            GfVec3d axis_X(1, 0, 0);
            GfVec3d axis_Y(0, 1, 0);
            if (auto attr = body.prim.GetAttribute(TfToken("xformOp:rotateXYZ")))
            {
                GfVec3f rotateXYZ;
                bool ok = attr.Get(&rotateXYZ);
                if (ok)
                {
                    GfRotation r = GfRotation(GfVec3d(1, 0, 0), rotateXYZ[0]) * GfRotation(GfVec3d(0, 1, 0), rotateXYZ[1]) *
                                   GfRotation(GfVec3d(0, 0, 1), rotateXYZ[2]);
                    GfMatrix3d m(r);
                    axis_X = axis_X * m;
                    axis_Y = axis_Y * m;
                }
            }

            if (body.pick_constraint_0)
                body.pick_constraint_0->setPivotB(btVector3(translate[0], translate[1], translate[2]));

            if (body.pick_constraint_X)
                body.pick_constraint_X->setPivotB(btVector3(translate[0] + axis_X[0], translate[1] + axis_X[1], translate[2] + axis_X[2]));

            if (body.pick_constraint_Y)
                body.pick_constraint_Y->setPivotB(btVector3(translate[0] + axis_Y[0], translate[1] + axis_Y[1], translate[2] + axis_Y[2]));
            bullet_scene_updated = true;
        }
        else if (attr_name == TfToken("xformOp:scale"))
        {
            if (auto attr = body.prim.GetAttribute(TfToken("xformOp:scale")))
            {
                GfVec3f scale;
                bool ok = attr.Get(&scale);
                if (ok)
                {
                    body.shape->setLocalScaling(btVector3(scale[0], scale[1], scale[2]));
                    bullet_scene_updated = true;
                }
            }
        }
#endif
    }
    else if (body.type == BodyType::STATIC)
    {
        const auto transform_matrix = xform_cache.GetLocalToWorldTransform(body.prim);
        GfTransform transform(transform_matrix);

        btTransform bullet_transform;
        bullet_transform.setIdentity();
        GfVec3d translate = transform.GetTranslation();
        bullet_transform.setOrigin(btVector3(translate[0], translate[1], translate[2]));

        GfQuatd q = transform.GetRotation().GetQuat();
        q.Normalize();
        bullet_transform.setRotation(btQuaternion(q.GetImaginary()[0], q.GetImaginary()[1], q.GetImaginary()[2], q.GetReal()));

        GfVec3d scale = transform.GetScale();
        bullet_transform.setOrigin(btVector3(translate[0], translate[1], translate[2]));
        body.shape->setLocalScaling(btVector3(btScalar(scale[0]), btScalar(scale[1]), btScalar(scale[2])));

        body.rigid_body->setWorldTransform(bullet_transform);
        bullet_scene_updated = true;

#ifdef UNUSED_OLD_VARIANT
        bool need_to_set_transform = false;
        btTransform transform = body.body->getWorldTransform();
        if (attr_name == TfToken("xformOp:translate"))
        {
            if (auto attr = body.prim.GetAttribute(TfToken("xformOp:translate")))
            {
                GfVec3d translate;
                bool ok = attr.Get(&translate);
                if (ok)
                    transform.setOrigin(btVector3(translate[0], translate[1], translate[2]));
            }
            need_to_set_transform = true;
        }
        else if (attr_name == TfToken("xformOp:rotateXYZ"))
        {
            if (auto attr = body.prim.GetAttribute(TfToken("xformOp:rotateXYZ")))
            {
                GfVec3f rotateXYZ;
                bool ok = attr.Get(&rotateXYZ);
                if (ok)
                    transform.setRotation(to_quaternion(rotateXYZ));
            }
            need_to_set_transform = true;
        }
        else if (attr_name == TfToken("xformOp:scale"))
        {
            if (auto attr = body.prim.GetAttribute(TfToken("xformOp:scale")))
            {
                GfVec3f scale;
                bool ok = attr.Get(&scale);
                if (ok)
                {
                    body.shape->setLocalScaling(btVector3(scale[0], scale[1], scale[2]));
                    bullet_scene_updated = true;
                }
            }
        }

        if (need_to_set_transform)
        {
            body.body->setWorldTransform(transform);
            bullet_scene_updated = true;
        }
#endif
    }
}

void BulletPhysicsEngine::update_data_in_bullet(std::unordered_map<SdfPath, ComponentsSet, SdfPath::Hash> children, bool& bullet_scene_updated)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    bullet_scene_updated = false;
    SdfPathVector path_to_remove;
    for (auto& child_paths : children)
    {
        auto& bullet_prim_path = child_paths.first;
        if (!m_stage->GetPrimAtPath(bullet_prim_path).IsValid())
            path_to_remove.push_back(bullet_prim_path);
        else
        {
            auto it = m_bodies.find(bullet_prim_path.GetPrimPath());
            if (it == m_bodies.end())
                continue;

            update_transforms_in_bullet(it->second, bullet_scene_updated);

            if (child_paths.second.size() > 0)
                update_children(m_stage->GetPrimAtPath(bullet_prim_path), it->second.shape.get(), child_paths.second);
        }
    }
    remove_objects(path_to_remove);

    std::unordered_map<SdfPath, std::unique_ptr<btCollisionShape>, SdfPath::Hash> a;
}

void BulletPhysicsEngine::update_gravity_direction()
{
    TfToken up = UsdGeomGetStageUpAxis(m_stage);
    if (up == UsdGeomTokens->x)
        m_gravity = { -m_options.gravity, 0, 0 };
    else if (up == UsdGeomTokens->y)
        m_gravity = { 0, -m_options.gravity, 0 };
    else if (up == UsdGeomTokens->z)
        m_gravity = { 0, 0, -m_options.gravity };
    else
        m_gravity = { 0, -m_options.gravity, 0 };
}

// TODO : speedup it
SdfPath BulletPhysicsEngine::parent_object(const SdfPath& chaild)
{
    for (auto& path : m_bodies_sorted_paths.GetIds())
        if (chaild.HasPrefix(path))
            return path;
    return SdfPath();
}

void BulletPhysicsEngine::on_objects_changed(UsdNotice::ObjectsChanged const& notice, UsdStageWeakPtr const& sender)
{
    if (m_miss_objects_changed || m_bodies.size() == 0)
        return;

    m_miss_objects_changed = true;

    using PathRange = UsdNotice::ObjectsChanged::PathRange;
    const PathRange paths_to_resync = notice.GetResyncedPaths();
    const PathRange paths_to_update = notice.GetChangedInfoOnlyPaths();

    std::unordered_map<SdfPath, ComponentsSet, SdfPath::Hash> children;
    HdPrimGather gather;
    SdfPathVector bodies_paths;

    auto travers = [&](const SdfPath& path) {
        bodies_paths.clear();
        gather.Subtree(m_bodies_sorted_paths.GetIds(), path.GetPrimPath(), &bodies_paths);
        if (bodies_paths.size() > 0)
        {
            for (const auto& body_paths : bodies_paths)
                children[body_paths] = ComponentsSet();
        }
        else
        {
            auto parent_object_path = parent_object(path);
            if (parent_object_path.IsEmpty())
                return;

            children[parent_object_path].insert(path);
        }
    };

    for (PathRange::const_iterator path = paths_to_resync.begin(); path != paths_to_resync.end(); ++path)
    {
        travers(path->GetPrimPath());
    }

    for (PathRange::const_iterator path = paths_to_update.begin(); path != paths_to_update.end(); ++path)
    {
        travers(path->GetPrimPath());
    }

    if (m_need_to_update_pick_constrins)
        update_pick_constraints();

    if (m_picked_dyn_bodies.size() * num_pick_constraints_per_object != m_dynamics_world->getNumConstraints())
    {

        m_miss_objects_changed = false;
        return;
    }

    bool bullet_scene_updated = false;
    update_data_in_bullet(children, bullet_scene_updated);

    if (bullet_scene_updated)
    {
        if (m_picked_dyn_bodies.size() * num_pick_constraints_per_object == m_dynamics_world->getNumConstraints())
        {
            m_dynamics_world->stepSimulation((float)m_options.num_subtiles / 60.0, m_options.num_subtiles);
            update_data_in_stage();
        }
        else
        {
            OPENDCC_ERROR("Coding error: picked_dyn_bodies.size() * num_pick_constraints_per_object  != dynamics_world->getNumConstraints()");
        }
    }

    m_miss_objects_changed = false;
}

void BulletPhysicsEngine::Options::read_from_settings(const std::string& prefix)
{
    static BulletPhysicsEngine::Options default_options;
    auto settings = Application::instance().get_settings();
    gravity = settings->get(prefix + ".gravity", default_options.gravity);
    pick_constraint_tau = settings->get(prefix + ".pick_constraint_tau", default_options.pick_constraint_tau);
    pick_constraint_impulse_clamp = settings->get(prefix + ".pick_constraint_impulse_clamp", default_options.pick_constraint_impulse_clamp);
    friction = settings->get(prefix + ".friction", default_options.friction);
    restitution = settings->get(prefix + ".restitution", default_options.restitution);
    linear_damping = settings->get(prefix + ".linear_damping", default_options.linear_damping);
    angular_damping = settings->get(prefix + ".angular_damping", default_options.angular_damping);
    num_subtiles = settings->get(prefix + ".num_substeps", default_options.num_subtiles);
}

OPENDCC_NAMESPACE_CLOSE
