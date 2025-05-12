// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/wrap_viewport_view.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include "opendcc/app/viewport/viewport_view.h"

OPENDCC_NAMESPACE_OPEN
using namespace pybind11;

void py_interp::bind::wrap_viewport_view(pybind11::module& m)
{
    class_<ViewportView, std::shared_ptr<ViewportView>>(m, "ViewportView")
        .def("pick_single_prim", &ViewportView::pick_single_prim)
        .def("set_rollover_prim", &ViewportView::set_rollover_prim)
        .def("look_through", &ViewportView::look_through)
        .def("get_scene_context_type", &ViewportView::get_scene_context_type)
        .def_static("get_render_plugins", &ViewportView::get_render_plugins)
        .def_static("get_render_display_name", &ViewportView::get_render_display_name);
}
OPENDCC_NAMESPACE_CLOSE
