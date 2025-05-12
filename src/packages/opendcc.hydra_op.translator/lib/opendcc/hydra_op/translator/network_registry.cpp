// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "network_registry.h"
#include "opendcc/hydra_op/schema/nodegraph.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

HydraOpNetworkRegistry::HydraOpNetworkRegistry(UsdStageRefPtr stage)
    : m_stage(stage)
{
}

std::shared_ptr<HydraOpNetwork> HydraOpNetworkRegistry::request_network(const PXR_NS::SdfPath& path)
{
    if (!m_stage || path.IsEmpty())
        return nullptr;

    auto network_it = m_networks.find(path);
    if (network_it == m_networks.end())
    {
        // If specified path is a node under network root then try to find the root
        auto tmp = path.GetParentPath();
        while (!tmp.IsAbsoluteRootPath())
        {
            network_it = m_networks.find(tmp);
            if (network_it != m_networks.end())
            {
                return network_it->second;
            }
            tmp = tmp.GetParentPath();
        }

        // Failed to find network, create one
        if (auto network = make_network(path))
        {
            m_networks[network->get_root()] = network;
            return network;
        }
        // Failed to create network, return null
        return nullptr;
    }
    return network_it->second;
}

std::shared_ptr<HydraOpNetwork> HydraOpNetworkRegistry::make_network(const PXR_NS::SdfPath& path)
{
    auto cur_prim = m_stage->GetPrimAtPath(path);
    while (cur_prim)
    {
        if (auto graph_prim = UsdHydraOpNodegraph(cur_prim))
        {
            return std::make_shared<HydraOpNetwork>(graph_prim);
        }

        cur_prim = cur_prim.GetParent();
    }
    return nullptr;
}

OPENDCC_NAMESPACE_CLOSE
