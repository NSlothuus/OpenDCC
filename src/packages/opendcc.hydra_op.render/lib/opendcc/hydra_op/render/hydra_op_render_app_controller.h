/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/usd/render/render_app_controller.h"

OPENDCC_NAMESPACE_OPEN

const inline RenderAppOption s_view_node_opt { "--view_node", "USD prim path to prim under HydraOpNodegraph" };

class HydraOpRenderAppController final : public RenderAppController
{
public:
    int process_args(const CLI::App& app) override;
};

OPENDCC_NAMESPACE_CLOSE
