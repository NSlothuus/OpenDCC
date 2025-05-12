// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd_editor/bullet_physics/session.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include "opendcc/usd_editor/bullet_physics/engine.h"
#include "opendcc/usd_editor/bullet_physics/entry_point.h"
#include "opendcc/usd_editor/bullet_physics/utils.h"

OPENDCC_NAMESPACE_OPEN

namespace
{
    BulletPhysicsSession& session()
    {
        return BulletPhysicsSession::instance();
    }

    void set_debug_draw_enabled(bool enable)
    {
        BulletPhysicsViewportUIExtension::set_enabled(enable);
    }

    bool is_debug_draw_enabled()
    {
        return BulletPhysicsViewportUIExtension::is_enabled();
    }
}

PYBIND11_MODULE(bullet_physics, m)
{
    using namespace pybind11;

    enum_<BulletPhysicsEngine::MeshApproximationType>(m, "MeshApproximationType")
        .value("NONE", BulletPhysicsEngine::MeshApproximationType::NONE)
        .value("BOX", BulletPhysicsEngine::MeshApproximationType::BOX)
        .value("CONVEX_HULL", BulletPhysicsEngine::MeshApproximationType::CONVEX_HULL)
        .value("VHACD", BulletPhysicsEngine::MeshApproximationType::VHACD)
        .export_values();

    m.def("session", session, return_value_policy::reference);

    class_<BulletPhysicsEngine, std::shared_ptr<BulletPhysicsEngine>>(m, "BulletPhysicsEngine")
        .def("create_selected_prims_as_static_object", &BulletPhysicsEngine::create_selected_prims_as_static_object)
        .def("create_selected_prims_as_dynamic_object", &BulletPhysicsEngine::create_selected_prims_as_dynamic_object)
        .def("remove_selected_prims_from_dym_scene", &BulletPhysicsEngine::remove_selected_prims_from_dym_scene)
        .def("remove_all", &BulletPhysicsEngine::remove_all)
        .def("print_state", &BulletPhysicsEngine::print_state)
        .def("step_simulation", &BulletPhysicsEngine::step_simulation)
        .def("update_solver_options", &BulletPhysicsEngine::update_solver_options);

    class_<BulletPhysicsSession, std::unique_ptr<BulletPhysicsSession, nodelete>>(m, "BulletPhysicsSession")
        .def("current_engine", &BulletPhysicsSession::current_engine)
        .def("is_enabled", &BulletPhysicsSession::is_enabled)
        .def("set_enabled", &BulletPhysicsSession::set_enabled);

    m.def("set_debug_draw_enabled", &set_debug_draw_enabled);
    m.def("is_debug_draw_enabled", &is_debug_draw_enabled);
    m.def("get_supported_prims_paths_recursively", &get_supported_prims_paths_recursively);
    m.def("reset_pivots", &reset_pivots);
}

OPENDCC_NAMESPACE_CLOSE
