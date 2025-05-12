// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/wrap_viewport_context_menu.h"
#include "opendcc/base/pybind_bridge/shiboken.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include "opendcc/app/viewport/viewport_context_menu_registry.h"
#include <QMenu>
#include <QContextMenuEvent>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

using namespace pybind11;

void py_interp::bind::wrap_viewport_context_menu(pybind11::module& m)
{
    class_<ViewportContextMenuRegistry, std::unique_ptr<ViewportContextMenuRegistry, nodelete>>(m, "ViewportContextMenuRegistry")
        .def_static("instance", &ViewportContextMenuRegistry::instance, return_value_policy::reference)
        .def("register_menu", [](ViewportContextMenuRegistry* self, const TfToken& context_type,
                                 ViewportContextMenuRegistry::CreateContextMenuFn creator) { return self->register_menu(context_type, creator); })
        .def("unregister_menu", &ViewportContextMenuRegistry::unregister_menu)
        .def("create_menu", &ViewportContextMenuRegistry::create_menu);
}
OPENDCC_NAMESPACE_CLOSE
