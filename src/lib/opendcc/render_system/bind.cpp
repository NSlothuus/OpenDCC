// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/opendcc.h"
#include "opendcc/base/pybind_bridge/pybind11.h"
#include "opendcc/render_system/render_factory.h"
#include "opendcc/render_system/render_system.h"

OPENDCC_NAMESPACE_OPEN
using namespace render_system;
using namespace pybind11;

PYBIND11_MODULE(rendersystem, m)
{
    enum_<RenderStatus>(m, "RenderStatus")
        .value("FAILED", RenderStatus::FAILED)
        .value("NOT_STARTED", RenderStatus::NOT_STARTED)
        .value("IN_PROGRESS", RenderStatus::IN_PROGRESS)
        .value("RENDERING", RenderStatus::RENDERING)
        .value("FINISHED", RenderStatus::FINISHED)
        .value("STOPPED", RenderStatus::STOPPED)
        .value("PAUSED", RenderStatus::PAUSED)
        .export_values();

    enum_<RenderMethod>(m, "RenderMethod")
        .value("NONE", RenderMethod::NONE)
        .value("PREVIEW", RenderMethod::PREVIEW)
        .value("IPR", RenderMethod::IPR)
        .value("DISK", RenderMethod::DISK)
        .export_values();

    class_<IRenderControl, IRenderControlPtr>(m, "IRenderControl")
        .def("description", &IRenderControl::description)
        .def("set_attributes", &IRenderControl::set_attributes)
        .def("control_type", &IRenderControl::control_type)
        .def("init_render", &IRenderControl::init_render)
        .def("start_render", &IRenderControl::start_render)
        .def("pause_render", &IRenderControl::pause_render)
        .def("stop_render", &IRenderControl::stop_render)
        .def("update_render", &IRenderControl::update_render)
        .def("wait_render", &IRenderControl::wait_render)
        .def("set_resolver", &IRenderControl::set_resolver)
        .def("render_status", &IRenderControl::render_status)
        .def("render_method", &IRenderControl::render_method)
        //.def("finished", &IRenderControl::finished)
        .def("dump", &IRenderControl::dump);

    class_<RenderControlHub, std::unique_ptr<RenderControlHub, nodelete>>(m, "RenderControlHub")
        .def_static("instance", &RenderControlHub::instance, return_value_policy::reference)
        .def("add_render_control", &RenderControlHub::add_render_control)
        .def("get_controls", &RenderControlHub::get_controls, return_value_policy::reference);

    class_<RenderSystem, std::unique_ptr<RenderSystem, nodelete>>(m, "RenderSystem")
        .def_static("instance", &RenderSystem::instance, return_value_policy::reference)
        .def("load_plugin", &RenderSystem::load_plugin)
        .def("set_render_control", overload_cast<IRenderControlPtr>(&RenderSystem::set_render_control), arg("control"))
        .def("set_render_control", overload_cast<const std::string&>(&RenderSystem::set_render_control), arg("name"))
        //.def("finished", &RenderSystem::finished)
        .def("render_control", &RenderSystem::render_control)
        .def("init_render", &RenderSystem::init_render)
        .def("wait_render", &RenderSystem::wait_render)
        .def("stop_render", &RenderSystem::stop_render)
        .def("start_render", &RenderSystem::start_render)
        .def("pause_render", &RenderSystem::pause_render)
        .def("update_render", &RenderSystem::update_render)
        .def("get_render_method", &RenderSystem::get_render_method)
        .def("get_render_status", &RenderSystem::get_render_status)
        .def("dump_debug_output", &RenderSystem::dump_debug_output);

    class_<RenderFactory, std::unique_ptr<RenderFactory, nodelete>>(m, "RenderFactory")
        .def_static("instance", &RenderFactory::instance, return_value_policy::reference)
        .def("create", &RenderFactory::create)
        .def("available_renders", &RenderFactory::available_renders);
}

OPENDCC_NAMESPACE_CLOSE
