// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "usd_fallback_proxy/cycles/ndrCycles/discovery.h"
#include <pxr/base/tf/stringUtils.h>
#include <pxr/base/arch/fileSystem.h>
#include <pxr/base/tf/getenv.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/base/plug/registry.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/base/tf/staticTokens.h>
#include <pxr/usd/usdShade/shader.h>
#include "usd_fallback_proxy/cycles/ndrCycles/node_definitions.h"

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

TF_DEFINE_PRIVATE_TOKENS(ndr_tokens, ((family, "shader"))((discoveryType, "cycles"))((sourceType, "cycles")));

NDR_REGISTER_DISCOVERY_PLUGIN(NdrCyclesDiscoveryPlugin)

NdrCyclesDiscoveryPlugin::NdrCyclesDiscoveryPlugin() {}

const NdrStringVec& NdrCyclesDiscoveryPlugin::GetSearchURIs() const
{
    static const auto result = []() -> NdrStringVec {
        NdrStringVec result = TfStringSplit(TfGetenv("CYCLES_PLUGIN_PATH"), ARCH_PATH_LIST_SEP);
        result.push_back("<built-in>");
        return result;
    }();
    return result;
}

NdrNodeDiscoveryResultVec NdrCyclesDiscoveryPlugin::DiscoverNodes(const Context& context)
{
    NdrNodeDiscoveryResultVec result;
    const auto node_definitions = get_node_definitions();
    if (!node_definitions)
        return result;

    for (const auto& prim : node_definitions->Traverse())
    {
        auto shader = UsdShadeShader(prim);
        if (!shader)
            continue;
        const auto shader_name = prim.GetName();
        NdrNodeDiscoveryResult node_discovery_result(NdrIdentifier(TfStringPrintf("cycles:%s", shader_name.GetText())), NdrVersion(1, 0),
                                                     shader_name.GetString(), ndr_tokens->family, ndr_tokens->discoveryType, ndr_tokens->sourceType,
                                                     "<built-in>", // uri
                                                     "<built-in>" // resolved uri
        );
        result.emplace_back(std::move(node_discovery_result));
    }
    return result;
}

NdrCyclesDiscoveryPlugin::~NdrCyclesDiscoveryPlugin() {}

OPENDCC_NAMESPACE_CLOSE
