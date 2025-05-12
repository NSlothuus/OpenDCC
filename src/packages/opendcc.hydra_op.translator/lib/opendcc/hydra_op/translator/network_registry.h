/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "network.h"
#include <pxr/usd/usd/stage.h>
#include "opendcc/hydra_op/translator/network.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_HYDRA_OP_TRANSLATOR_API HydraOpNetworkRegistry final
{
public:
    HydraOpNetworkRegistry(PXR_NS::UsdStageRefPtr stage);
    HydraOpNetworkRegistry(const HydraOpNetworkRegistry&) = delete;
    HydraOpNetworkRegistry(HydraOpNetworkRegistry&&) = delete;
    HydraOpNetworkRegistry& operator=(const HydraOpNetworkRegistry&) = delete;
    HydraOpNetworkRegistry& operator=(HydraOpNetworkRegistry&&) = delete;

    std::shared_ptr<HydraOpNetwork> request_network(const PXR_NS::SdfPath& path);

private:
    std::shared_ptr<HydraOpNetwork> make_network(const PXR_NS::SdfPath& path);

    PXR_NS::UsdStageRefPtr m_stage;

    std::unordered_map<PXR_NS::SdfPath, std::shared_ptr<HydraOpNetwork>, PXR_NS::SdfPath::Hash> m_networks;
};

OPENDCC_NAMESPACE_CLOSE
