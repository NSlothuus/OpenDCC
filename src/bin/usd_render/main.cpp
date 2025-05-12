// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/render/render_app.h"

OPENDCC_NAMESPACE_USING
PXR_NAMESPACE_USING_DIRECTIVE

int main(int argc, char** argv)
{
    auto cli_app = std::make_unique<CLI::App>("Options");
    UsdRenderApp app(std::move(cli_app), TfToken("USD"));
    return app.exec(argc, argv);
}
