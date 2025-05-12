// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/opendcc.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include <pxr/base/tf/pyContainerConversions.h>
#include "opendcc/anim_engine/core/engine.h"
#include "opendcc/anim_engine/core/session.h"

OPENDCC_NAMESPACE_OPEN

namespace
{
    AnimEngineSession& anim_session()
    {
        return AnimEngineSession::instance();
    }
}

PYBIND11_MODULE(_anim_engine_core, m)
{
    using namespace pybind11;

    class_<AnimEngine::CurveId>(m, "CurveId");

    enum_<AnimEngine::AttributesScope>(m, "AttributesScope")
        .value("TRANSLATE", AnimEngine::AttributesScope::TRANSLATE)
        .value("ROTATE", AnimEngine::AttributesScope::ROTATE)
        .value("SCALE", AnimEngine::AttributesScope::SCALE)
        .value("ALL", AnimEngine::AttributesScope::ALL)
        .export_values();

    class_<AnimEngine, std::shared_ptr<AnimEngine>>(m, "AnimEngine")
        .def("key_attribute", (AnimEngine::CurveIdsList(AnimEngine::*)(PXR_NS::UsdAttribute)) & AnimEngine::key_attribute)
        .def("key_attributes", (AnimEngine::CurveIdsList(AnimEngine::*)(const PXR_NS::UsdAttributeVector&)) & AnimEngine::key_attributes)
        .def("key_attributes",
             (AnimEngine::CurveIdsList(AnimEngine::*)(const PXR_NS::UsdAttributeVector&, std::vector<uint32_t>)) & AnimEngine::key_attributes)
        .def("remove_animation_curve", (bool(AnimEngine::*)(PXR_NS::UsdAttribute)) & AnimEngine::remove_animation_curves)
        .def("remove_animation_curves", (bool(AnimEngine::*)(const PXR_NS::UsdAttributeVector&)) & AnimEngine::remove_animation_curves)
        .def("bake", (bool(AnimEngine::*)(PXR_NS::SdfLayerRefPtr, const PXR_NS::SdfPathVector&, const PXR_NS::UsdAttributeVector&, double, double,
                                          const std::vector<double>&, bool)) &
                         AnimEngine::bake)
        .def("bake_all", &AnimEngine::bake_all)
        .def("create_animation_on_selected_prims", &AnimEngine::create_animation_on_selected_prims)
        .def("is_attribute_animated", (bool(AnimEngine::*)(PXR_NS::UsdAttribute) const) & AnimEngine::is_attribute_animated)
        .def("is_attribute_animated", (bool(AnimEngine::*)(PXR_NS::UsdAttribute, uint32_t) const) & AnimEngine::is_attribute_animated)
        .def("is_prim_has_animated_attributes", &AnimEngine::is_prim_has_animated_attributes)
        .def("is_save_on_current_layer", &AnimEngine::is_save_on_current_layer)
        .def("set_save_on_current_layer", &AnimEngine::set_save_on_current_layer);

    m.def("session", &anim_session, return_value_policy::reference);

    class_<AnimEngineSession, std::unique_ptr<AnimEngineSession, nodelete>>(m, "AnimEngineSession")
        .def("current_engine", &AnimEngineSession::current_engine);
}
OPENDCC_NAMESPACE_CLOSE
