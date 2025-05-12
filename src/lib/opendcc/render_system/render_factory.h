/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/render_system/api.h"
#include "opendcc/render_system/irender.h"
#include <memory>
#include <unordered_map>
#include <unordered_set>

OPENDCC_NAMESPACE_OPEN

namespace render_system
{
    /**
     * @brief Factory class for creating render objects.
     *
     * The RenderFactory class is designed for registering and creating render objects.
     * It uses a registry of render aliases and corresponding creator functions for building different
     * types of render.
     *
     * @note This class follows the Singleton design pattern.
     */
    class OPENDCC_RENDER_SYSTEM_API RenderFactory
    {
        std::unordered_map<std::string, std::function<RenderPtr()>> m_registry;

        RenderFactory() = default;
        RenderFactory(RenderFactory&) = delete;
        RenderFactory(RenderFactory&&) = delete;
        RenderFactory& operator=(RenderFactory&) = delete;
        RenderFactory& operator=(RenderFactory&&) = delete;

    public:
        /**
         * @brief Returns the instance of the RenderFactory.
         * @return The instance of the RenderFactory.
         */
        static RenderFactory& instance();

        /**
         * @brief Registers a render with its corresponding creator function.
         * @param render_alias The alias of the render.
         * @param render_name The name of the render.
         * @param creator The creator function for creating the render object.
         * @return True if the render was registered successfully, false otherwise.
         */
        bool register_render(const std::string& render_alias, const std::string& render_name, std::function<RenderPtr()> creator);

        /**
         * @brief Adds a render with the specified name to the render factory.
         * @param render_name The name of the render to create.
         * @return The created render, or nullptr if creation fails.
         */
        RenderPtr create(const std::string& render_name);

        /**
         * @brief Retrieves the set of available renders.
         * @return The available renders.
         */
        std::unordered_set<std::string> available_renders();
    };

}
#define DefineRender(RenderAlias, RenderClass)                                                  \
    bool Is##RenderClass##Available = render_system::RenderFactory::instance().register_render( \
        RenderAlias, #RenderClass, [] { return std::shared_ptr<render_system::IRender>(new RenderClass()); });

OPENDCC_NAMESPACE_CLOSE
