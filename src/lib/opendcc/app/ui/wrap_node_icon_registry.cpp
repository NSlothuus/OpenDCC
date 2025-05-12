// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/ui/wrap_node_icon_registry.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include "opendcc/base/pybind_bridge/shiboken.h"
#include "opendcc/app/ui/node_icon_registry.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE;

using namespace pybind11;

void py_interp::bind::wrap_node_icon_registry(pybind11::module& m)
{
    enum_<IconFlags>(m, "IconFlags").value("NONE", IconFlags::NONE).value("NOT_ON_EDIT_TARGET", IconFlags::NOT_ON_EDIT_TARGET);

    class_<NodeIconRegistry>(m, "NodeIconRegistry")
        .def_static("instance", &NodeIconRegistry::instance, return_value_policy::reference)
        .def("register_icon",
             overload_cast<const TfToken&, const std::string&, const std::string&, const std::string&>(&NodeIconRegistry::register_icon),
             arg("context_type"), arg("node_type"), arg("icon_path"), arg("svg_path") = "")
        .def("register_icon", overload_cast<const TfToken&, const std::string&, const QPixmap&, const std::string&>(&NodeIconRegistry::register_icon),
             arg("context_type"), arg("node_type"), arg("pixmap"), arg("svg_path") = "")
        .def("unregister_icon", &NodeIconRegistry::unregister_icon)
        .def("get_icon", &NodeIconRegistry::get_icon, "context_type"_a, "node_type"_a, "flags"_a = IconFlags::NONE)
        .def("get_svg", &NodeIconRegistry::get_svg, "context_type"_a, "node_type"_a, "flags"_a = IconFlags::NONE);
}
OPENDCC_NAMESPACE_CLOSE
