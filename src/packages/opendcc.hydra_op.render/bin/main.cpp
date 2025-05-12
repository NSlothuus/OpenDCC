// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/render/render_app.h"
#include "opendcc/hydra_op/render/hydra_op_render_app_controller.h"

OPENDCC_NAMESPACE_USING
PXR_NAMESPACE_USING_DIRECTIVE

int main(int argc, char** argv)
{
    auto cli_app = std::make_unique<CLI::App>("Options");
    cli_app->add_option(s_view_node_opt.name, s_view_node_opt.description);
    UsdRenderApp app(std::move(cli_app), TfToken("HydraOp"));
    return app.exec(argc, argv);
}
