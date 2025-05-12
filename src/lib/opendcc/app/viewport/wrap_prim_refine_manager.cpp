// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/wrap_prim_refine_manager.h"
#include "opendcc/base/pybind_bridge/shiboken.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include "opendcc/app/viewport/viewport_refine_manager.h"

OPENDCC_NAMESPACE_OPEN

void py_interp::bind::wrap_prim_refine_manager(pybind11::module& m)
{
    using namespace pybind11;

    class_<UsdViewportRefineManager>(m, "UsdViewportRefineManager")
        .def_static("instance", &UsdViewportRefineManager::instance, return_value_policy::reference)
        .def("set_refine_level",
             (void(UsdViewportRefineManager::*)(const PXR_NS::UsdStageRefPtr&, const PXR_NS::SdfPath&, int)) &
                 UsdViewportRefineManager::set_refine_level,
             arg("stage"), arg("prim_path"), arg("refine_level"))
        .def("set_refine_level",
             (void(UsdViewportRefineManager::*)(const PXR_NS::UsdStageCache::Id&, const PXR_NS::SdfPath&, int)) &
                 UsdViewportRefineManager::set_refine_level,
             arg("stage_id"), arg("prim_path"), arg("refine_level"))
        .def("get_refine_level",
             (int(UsdViewportRefineManager::*)(const PXR_NS::UsdStageRefPtr&, const PXR_NS::SdfPath&) const) &
                 UsdViewportRefineManager::get_refine_level,
             arg("stage"), arg("prim_path"))
        .def("get_refine_level",
             (int(UsdViewportRefineManager::*)(const PXR_NS::UsdStageCache::Id&, const PXR_NS::SdfPath&) const) &
                 UsdViewportRefineManager::get_refine_level,
             arg("stage_id"), arg("prim_path"))
        .def("clear_all", &UsdViewportRefineManager::clear_all)
        .def("clear_stage", (void(UsdViewportRefineManager::*)(const PXR_NS::UsdStageRefPtr&)) & UsdViewportRefineManager::clear_stage, arg("stage"))
        .def("clear_stage", (void(UsdViewportRefineManager::*)(const PXR_NS::UsdStageCache::Id&)) & UsdViewportRefineManager::clear_stage,
             arg("stage_id"))
        .def("clear_refine_level",
             (void(UsdViewportRefineManager::*)(const PXR_NS::UsdStageCache::Id&, const PXR_NS::SdfPath&)) &
                 UsdViewportRefineManager::clear_refine_level,
             arg("stage_id"), arg("prim_path"))
        .def("clear_refine_level",
             (void(UsdViewportRefineManager::*)(const PXR_NS::UsdStageRefPtr&, const PXR_NS::SdfPath&)) &
                 UsdViewportRefineManager::clear_refine_level,
             arg("stage"), arg("prim_path"));
}

OPENDCC_NAMESPACE_CLOSE
