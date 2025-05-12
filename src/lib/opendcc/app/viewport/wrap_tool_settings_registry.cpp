// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/wrap_tool_settings_registry.h"
#include "opendcc/base/pybind_bridge/shiboken.h"
#include "opendcc/base/pybind_bridge/usd.h"
#include "opendcc/app/viewport/tool_settings_view.h"
#include "opendcc/app/viewport/iviewport_tool_context.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE
using namespace pybind11;

void py_interp::bind::wrap_tool_settings_registry(pybind11::module& m)
{
    class_<ToolSettingsViewRegistry, std::unique_ptr<ToolSettingsViewRegistry, nodelete>>(m, "ToolSettingsViewRegistry")
        .def_static("instance", &ToolSettingsViewRegistry::instance, return_value_policy::reference)
        .def_static("register_tool_settings_view",
                    [](const TfToken& name, const TfToken& context, std::function<QWidget*()> factory_fn) {
                        return ToolSettingsViewRegistry::register_tool_settings_view(name, context, factory_fn);
                    })
        .def_static("unregister_tool_settings_view", &ToolSettingsViewRegistry::unregister_tool_settings_view);
}

OPENDCC_NAMESPACE_CLOSE
