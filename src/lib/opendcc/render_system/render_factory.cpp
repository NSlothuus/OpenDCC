// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/render_system/render_factory.h"

OPENDCC_NAMESPACE_OPEN

namespace render_system
{
    RenderFactory& RenderFactory::instance()
    {
        static RenderFactory inst;
        return inst;
    }

    bool RenderFactory::register_render(const std::string& render_alias, const std::string& render_name, std::function<RenderPtr()> creator)
    {
        auto place = m_registry.find(render_name);
        if (place != m_registry.end())
            return false;

        m_registry[render_alias] = creator;
        m_registry[render_name] = creator;
        return true;
    }

    std::shared_ptr<IRender> RenderFactory::create(const std::string& render_name)
    {
        auto place = m_registry.find(render_name);
        if (place == m_registry.end())
            return nullptr;

        return std::shared_ptr<IRender>(place->second());
    }

    std::unordered_set<std::string> RenderFactory::available_renders()
    {
        std::unordered_set<std::string> renders;
        for (auto item : m_registry)
            renders.emplace(item.first);
        return renders;
    }

}

OPENDCC_NAMESPACE_CLOSE
