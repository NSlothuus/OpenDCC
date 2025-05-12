// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "shader_node_registry.h"
#include "pxr/base/plug/registry.h"
#include "pxr/usd/ndr/discoveryPlugin.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

std::string OPENDCC_NAMESPACE::ShaderNodeRegistry::get_node_plugin_name(const PXR_NS::TfToken& node_name)
{
    auto& self = instance();
    self.init();

    auto iter = self.m_node_to_plugin.find(node_name);
    return iter == self.m_node_to_plugin.end() ? "" : iter->second;
}

PXR_NS::NdrNodeDiscoveryResultVec OPENDCC_NAMESPACE::ShaderNodeRegistry::get_ndr_plugin_nodes(const std::string& plugin_name)
{
    auto& self = instance();
    self.init();

    auto iter = self.m_plugin_nodes.find(plugin_name);
    return iter == self.m_plugin_nodes.end() ? NdrNodeDiscoveryResultVec() : iter->second;
}

OPENDCC_NAMESPACE::ShaderNodeRegistry::ShaderNodeRegistry()
{
    m_watcher = std::make_unique<PluginWatcher>();
    auto registry = PlugRegistry::FindTypeByName("NdrDiscoveryPlugin");
    registry.GetAllDerivedTypes(&m_ndr_plugins);
    for (const auto& plugin : m_ndr_plugins)
    {
        auto plug = PlugRegistry::GetInstance().GetPluginForType(plugin);
        if (plug)
            plug->Load();
    }
}

std::vector<std::string> OPENDCC_NAMESPACE::ShaderNodeRegistry::get_loaded_node_plugin_names()
{
    auto& self = instance();
    self.init();

    std::vector<std::string> result(self.m_loaded_plugins.size());
    std::transform(self.m_loaded_plugins.begin(), self.m_loaded_plugins.end(), result.begin(), [](const PluginEntry& plugin) { return plugin.name; });
    return result;
}

OPENDCC_NAMESPACE::ShaderNodeRegistry& ShaderNodeRegistry::instance()
{
    static ShaderNodeRegistry instance;
    return instance;
}

void OPENDCC_NAMESPACE::ShaderNodeRegistry::PluginWatcher::on_did_register_plugins(const PXR_NS::PlugNotice::DidRegisterPlugins& notice)
{
    auto& self = ShaderNodeRegistry::instance();
    self.m_loaded_plugins.clear();
    auto registry = PlugRegistry::FindTypeByName("NdrDiscoveryPlugin");
    registry.GetAllDerivedTypes(&self.m_ndr_plugins);
    self.init();
}

void OPENDCC_NAMESPACE::ShaderNodeRegistry::init()
{
    std::set<PluginEntry> loaded_plugins;
    for (const auto& plugin : m_ndr_plugins)
    {
        auto plug = PlugRegistry::GetInstance().GetPluginForType(plugin);
        if (plug && plug->IsLoaded())
        {
            const PluginEntry entry = { plugin, plug->GetName() };
            loaded_plugins.emplace(std::move(entry));
        }
    }
    if (loaded_plugins == m_loaded_plugins)
        return;

    m_node_to_plugin.clear();
    m_plugin_nodes.clear();
    m_loaded_plugins = loaded_plugins;

    class CustomCtx : public NdrDiscoveryPluginContext
    {
    public:
        CustomCtx() = default;

        virtual TfToken GetSourceType(const TfToken& discoveryType) const override { return TfToken(); }
    } ctx;

    for (const auto plugin_entry : m_loaded_plugins)
    {
        if (const auto discovery_plug_factory = plugin_entry.type.GetFactory<NdrDiscoveryPluginFactoryBase>())
        {
            const auto discovery_plug = discovery_plug_factory->New();
            auto nodes = discovery_plug->DiscoverNodes(ctx);
            if (!nodes.empty())
            {
                std::sort(nodes.begin(), nodes.end(),
                          [](const NdrNodeDiscoveryResult& left, const NdrNodeDiscoveryResult& right) { return left.name < right.name; });
                for (const auto& node : nodes)
                    m_node_to_plugin[node.identifier] = plugin_entry.name;
                m_plugin_nodes[plugin_entry.name] = std::move(nodes);
            }
        }
    }
}

ShaderNodeRegistry::PluginWatcher::PluginWatcher()
{
    TfNotice::Register(TfCreateWeakPtr(this), &PluginWatcher::on_did_register_plugins);
}

OPENDCC_NAMESPACE_CLOSE
