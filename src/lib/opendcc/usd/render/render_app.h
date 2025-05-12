/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd/render/api.h"
#include "opendcc/base/vendor/cli11/CLI11.hpp"
#include <pxr/base/tf/token.h>
#include <pxr/usd/usdUtils/timeCodeRange.h>
#include "opendcc/render_system/irender.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_USD_RENDER_API UsdRenderApp
{
public:
    UsdRenderApp(std::unique_ptr<CLI::App> app, const PXR_NS::TfToken& render_type);

    int exec(int argc, char* argv[]);

private:
    struct CommonOptions
    {
        std::string type;
        std::string transferred_layers;
        std::vector<std::string> frame;
        std::string stage_file;
    };

    struct CommonArgsHandling
    {
        std::vector<PXR_NS::UsdUtilsTimeCodeRange> time_ranges;
        render_system::RenderMethod render_method = render_system::RenderMethod::NONE;
        int process_status = 0;

        CommonArgsHandling() = default;
        CommonArgsHandling(int status)
            : process_status(status)
        {
        }
    };
    CommonArgsHandling handle_common_args();

    std::unique_ptr<CLI::App> m_app;
    PXR_NS::TfToken m_render_type;

    CommonOptions m_common_options;
};

OPENDCC_NAMESPACE_CLOSE
