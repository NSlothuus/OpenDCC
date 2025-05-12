/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "api.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"
#include <atomic>
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include "pxr/usd/usd/notice.h"
#include "opendcc/base/vendor/eventpp/eventdispatcher.h"
#include "opendcc/app/core/application.h"
#include "opendcc/usd_editor/bullet_physics/debug_drawer.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
struct btDbvtBroadphase;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionShape;
class btPoint2PointConstraint;
class btRigidBody;
class DebugDrawer;

OPENDCC_NAMESPACE_OPEN

class BULLET_PHYSICS_API BulletPhysicsEngine : public PXR_NS::TfWeakBase
{
public:
    enum class BodyType
    {
        NONE,
        STATIC,
        DYNAMIC
    };

    enum class MeshApproximationType
    {
        NONE,
        BOX,
        CONVEX_HULL,
        VHACD
    };

    BulletPhysicsEngine(const BulletPhysicsEngine&);
    BulletPhysicsEngine(PXR_NS::UsdStageRefPtr stage, double time);
    ~BulletPhysicsEngine();
    void step_simulation(bool add_gravity);
    void on_selection_changed();
    void deactivate();
    void activate();
    bool is_active() const;
    void create_selected_prims_as_static_object();
    void create_selected_prims_as_dynamic_object(MeshApproximationType mesh_approximation_type);
    void remove_selected_prims_from_dym_scene();
    void remove_all();
    void update_solver_options();
    void print_state() const;
    void set_debug_drawer(btIDebugDraw* debugDrawer);
    btIDebugDraw* get_debug_drawer() const;
    void draw_world();
    const static std::string s_extension_short_name;

private:
    using ComponentsSet = std::unordered_set<PXR_NS::SdfPath, PXR_NS::SdfPath::Hash>;

    struct BodyInfo
    {
        BodyInfo() {}
        BodyInfo(PXR_NS::SdfPath in_path, BodyType in_type, MeshApproximationType in_mesh_approximation_type)
            : path(in_path)
            , type(in_type)
            , mesh_approximation_type(in_mesh_approximation_type)
        {
        }

        PXR_NS::SdfPath path;
        BodyType type = BodyType::NONE;
        MeshApproximationType mesh_approximation_type = MeshApproximationType::NONE;
    };

    struct RigidBody : public BodyInfo
    {
        std::unique_ptr<btRigidBody> rigid_body;
        std::unique_ptr<btCollisionShape> shape;
        std::vector<std::unique_ptr<btPoint2PointConstraint>> pick_constraints;
        PXR_NS::UsdPrim prim;
    };

    struct Options
    {
        void read_from_settings(const std::string& prefix);
        float gravity = 50;
        float pick_constraint_tau = 1.0f;
        float pick_constraint_impulse_clamp = 6.0f;
        float friction = 1.0f;
        float restitution = 1.0f;
        float linear_damping = 0.99f;
        float angular_damping = 1.0f;
        uint32_t num_subtiles = 30;
    };

    std::vector<BodyInfo> remove_children_from_paths_list(const std::vector<BodyInfo>& in);
    void add_pick_constraints(RigidBody& body);
    void create_pick_constraints(const PXR_NS::SdfPathVector& paths);
    void remove_pick_constraints(RigidBody& body);
    void remove_pick_constraints();
    void update_pick_constraints();
    void on_objects_changed(PXR_NS::UsdNotice::ObjectsChanged const& notice, PXR_NS::UsdStageWeakPtr const& sender);
    // mesh_approximation_type use only if type == DYNAMIC
    void add_objects(const std::vector<BodyInfo>& bodys_info);
    void remove_objects(const PXR_NS::SdfPathVector& paths);
    void update_data_in_stage();
    void update_data_in_bullet(std::unordered_map<PXR_NS::SdfPath, ComponentsSet, PXR_NS::SdfPath::Hash> paths, bool& bullet_scene_updated);
    void update_transforms_in_bullet(RigidBody& body, bool& bullet_scene_updated);
    void update_gravity_direction();
    PXR_NS::SdfPathVector get_subtree_bodies_paths(const PXR_NS::SdfPathVector& interesting_paths, bool add_parrents = false,
                                                   bool add_children = true);
    PXR_NS::SdfPath parent_object(const PXR_NS::SdfPath& chaild);
    std::unordered_map<PXR_NS::SdfPath, RigidBody, PXR_NS::SdfPath::Hash> m_bodies;
    PXR_NS::SdfPathVector m_picked_dyn_bodies;
    PXR_NS::Hd_SortedIds m_bodies_sorted_paths;
    PXR_NS::TfNotice::Key m_objects_changed_notice_key;
    PXR_NS::UsdStageRefPtr m_stage;
    PXR_NS::GfVec3f m_gravity;
    double m_last_time = 0;
    std::unique_ptr<btDefaultCollisionConfiguration> m_collision_configuration;
    std::unique_ptr<btCollisionDispatcher> m_cillision_dispatcher;
    std::unique_ptr<btDbvtBroadphase> m_overlapping_pair_cache;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld> m_dynamics_world;
    std::vector<BodyInfo> m_deactivated_prims;
    bool m_miss_objects_changed = false;
    bool m_need_to_update_pick_constrins = false;
    bool m_is_active = true;
    std::mutex m_mutex;
    Options m_options;
};

using BulletPhysicsEnginePtr = std::shared_ptr<BulletPhysicsEngine>;

OPENDCC_NAMESPACE_CLOSE
