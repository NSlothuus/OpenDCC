// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "hydra_op_render_app_controller.h"
#include "opendcc/hydra_op/session.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

int HydraOpRenderAppController::process_args(const CLI::App& app)
{
    const auto view_node_str = app.get_option(s_view_node_opt.name)->as<std::string>();
    HydraOpSession::instance().set_view_node(SdfPath(view_node_str));
    return 0;
}

OPENDCC_NAMESPACE_CLOSE
