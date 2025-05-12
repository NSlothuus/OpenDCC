/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/render_system/api.h"
#include <functional>
#include <memory>
#include <variant>
#include <vector>
#include <string>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN
namespace render_system
{
    using RenderAttribute = std::variant<bool, int, float, std::string, std::vector<float>, std::vector<int>>;
    using RenderAttributes = std::unordered_map<std::string, RenderAttribute>;

    /**
     * @enum RenderMethod
     * @brief Enumerates the different rendering methods.
     *
     * This enum class defines the available rendering methods.
     */
    enum class RenderMethod
    {
        NONE, /**< No rendering method specified. */
        PREVIEW, /**< Renders a preview of the scene. */
        IPR, /**< Interactive Progressive Rendering. */
        DISK, /**< Renders the scene to disk. */
        DUMP, /**< Dumps the rendering data for debugging purposes. */
    };

    /**
     * @enum RenderStatus
     * @brief Enumerates the possible rendering statuses.
     *
     * This enum class defines the various statuses that a rendering process can have.
     */
    enum class RenderStatus
    {
        FAILED = -1, /**< The rendering process has failed. */
        NOT_STARTED = 1, /**< The rendering process has not yet started. */
        IN_PROGRESS, /**< The rendering process is currently in progress. */
        RENDERING, /**< The rendering process is actively rendering. */
        FINISHED, /**< The rendering process has finished successfully. */
        STOPPED, /**< The rendering process has been stopped. */
        PAUSED, /**< The rendering process has been paused. */
    };

    /**
     * @class IRender
     * @brief Interface for a rendering module.
     *
     * The IRender class defines the interface for a rendering module.
     * This interface provides methods for setting attributes, manipulating the render state
     * and registering a callback function for when rendering is finished.
     * It also includes a method for dumping rendering data to a specified output file path.
     */
    class OPENDCC_RENDER_SYSTEM_API IRender
    {
    public:
        /**
         * @brief Sets the rendering attributes.
         * @param attributes The rendering attributes to be set.
         */
        virtual void set_attributes(const RenderAttributes& attributes) = 0;

        /**
         * @brief Initializes the rendering process.
         * @param type The method to be used for rendering.
         * @return True if initialization is successful, false otherwise.
         */
        virtual bool init_render(RenderMethod type) = 0;

        /**
         * @brief Starts the rendering process.
         * @return True if rendering starts successfully, false otherwise.
         */
        virtual bool start_render() = 0;

        /**
         * @brief Pauses the rendering process.
         * @return True if rendering is successfully paused, false otherwise.
         */
        virtual bool pause_render() = 0;

        /**
         * @brief Resumes the rendering process.
         * @return True if rendering is successfully resumed, false otherwise.
         */
        virtual bool resume_render() = 0;

        /**
         * @brief Stops the rendering process.
         * @return True if rendering process has been stopped, false otherwise.
         */
        virtual bool stop_render() = 0;

        /**
         * @brief Updates the rendering process.
         */
        virtual void update_render() = 0;

        /**
         * @brief Waits for the rendering process to complete.
         */
        virtual void wait_render() = 0;

        /**
         * @brief Retrieves the current rendering status.
         * @return The current rendering status.
         */
        virtual RenderStatus render_status() = 0;
        virtual void finished(std::function<void(RenderStatus)> cb) = 0;

        /**
         * @brief Dumps the rendering data to the specified output file path.
         * @param output_file_path The path of the output file to dump the rendering data to.
         * @return True if dumping is successful, false otherwise.
         */
        virtual bool dump(const std::string& output_file_path) { return false; };

        virtual ~IRender() {}
    };

    typedef std::shared_ptr<IRender> RenderPtr;
}

OPENDCC_NAMESPACE_CLOSE
