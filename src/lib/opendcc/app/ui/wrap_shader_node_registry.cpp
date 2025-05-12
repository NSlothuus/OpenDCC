// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/ui/wrap_shader_node_registry.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include "opendcc/app/ui/shader_node_registry.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE
using namespace pybind11;

void py_interp::bind::wrap_shader_node_registry(pybind11::module& m)
{
    class_<ShaderNodeRegistry>(m, "ShaderNodeRegistry")
        .def_static("get_node_plugin_name", &ShaderNodeRegistry::get_node_plugin_name)
        .def_static("get_ndr_plugin_nodes", &ShaderNodeRegistry::get_ndr_plugin_nodes)
        .def_static("get_loaded_node_plugin_names", &ShaderNodeRegistry::get_loaded_node_plugin_names);
}

OPENDCC_NAMESPACE_CLOSE
