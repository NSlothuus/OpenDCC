/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/render_system/api.h"

#include "opendcc/render_system/irender.h"

OPENDCC_NAMESPACE_OPEN

namespace render_system
{
    struct OPENDCC_RENDER_SYSTEM_API IRenderControl
    {
        virtual std::string control_type() const = 0;
        virtual std::string description() { return {}; }
        virtual void set_attributes(const RenderAttributes& attributes) = 0;
        virtual bool init_render(RenderMethod type) = 0;
        virtual bool start_render() = 0;
        virtual bool pause_render() = 0;
        virtual bool resume_render() = 0;
        virtual bool stop_render() = 0;
        virtual void update_render() = 0;
        virtual void wait_render() = 0;
        virtual void set_resolver(const std::string&) = 0;
        virtual RenderStatus render_status() = 0;
        virtual RenderMethod render_method() const = 0;
        virtual void finished(std::function<void(RenderStatus)> cb) = 0;
        virtual bool dump(const std::string& output_file_path) { return false; };

        virtual ~IRenderControl() {}
    };
    using IRenderControlPtr = std::shared_ptr<IRenderControl>;

    /**
     * @brief Hub for managing render controllers.
     *
     * The RenderControlHub class provides a centralized hub for managing and accessing render controllers.
     * Render controllers can be added to the hub, and the hub allows access to the registered controllers.
     *
     * @note It follows the Singleton design pattern.
     */
    class OPENDCC_RENDER_SYSTEM_API RenderControlHub
    {
        RenderControlHub() = default;
        RenderControlHub(const RenderControlHub&) = delete;
        RenderControlHub(RenderControlHub&&) = delete;
        RenderControlHub operator=(const RenderControlHub&) = delete;
        RenderControlHub operator=(RenderControlHub&&) = delete;

    public:
        static RenderControlHub& instance();
        /**
         * @brief Adds a render controller to the hub.
         * @param control The render controller to be added.
         * @return True if render controller was successfully added, false otherwise.
         */
        bool add_render_control(IRenderControlPtr control);
        /**
         * @brief Retrieves the controller registered in the hub.
         * @return Set of controller.
         */
        const std::unordered_map<std::string, IRenderControlPtr>& get_controls() const;

    private:
        std::unordered_map<std::string, IRenderControlPtr> m_hub;
    };

    /**
     * @brief System for managing rendering operations.
     *
     * The RenderSystem class provides functionality for managing rendering operations.
     * It allows loading plugins, setting the render controller, registering a callback function
     * for rendering statuses and dumping debug output.
     */
    class OPENDCC_RENDER_SYSTEM_API RenderSystem
    {
        RenderSystem() = default;
        RenderSystem(const RenderSystem&) = delete;
        RenderSystem(RenderSystem&&) = delete;
        RenderSystem& operator=(const RenderSystem&) = delete;
        RenderSystem& operator=(RenderSystem&&) = delete;

    public:
        /**
         * @brief Returns the instance of the RenderSystem.
         * @return The instance of the RenderSystem.
         */
        static RenderSystem& instance();

        /**
         * @brief Loads a plugin from the specified path.
         * @param path The path of the plugin to load.
         * @return True if loading is successful, false otherwise.
         */
        bool load_plugin(const std::string& path);
        /**
         * @brief Sets the render controller.
         * @param control The render controller to be set.
         * @return True if the render controller was successfully set, false otherwise.
         */
        bool set_render_control(IRenderControlPtr control);
        /**
         * @brief Sets the render controller.
         * @param control_name The name of the render controller to be set.
         * @return True if the render controller was successfully set, false otherwise.
         */
        bool set_render_control(const std::string& control_name);
        /**
         * @brief Registers a callback function to be called when rendering is finished.
         * @param cb The callback function to be registered.
         */
        void finished(std::function<void(RenderStatus)> cb);
        /**
         * @brief Retrieves the render controller.
         * @return The render controller.
         */
        IRenderControlPtr render_control() { return m_render_control; }

        /**
         * @brief Initializes the rendering process with the specified method.
         * @param type The method to be used for rendering.
         * @return True if initialization is successful, false otherwise.
         */
        bool init_render(RenderMethod type);
        /**
         * @brief Starts the rendering process.
         * @return True if rendering starts successfully, false otherwise.
         */
        bool start_render();
        /**
         * @brief Pauses the rendering process.
         * @return True if rendering is successfully paused, false otherwise.
         */
        bool pause_render();
        /**
         * @brief Resumes the rendering process.
         * @return True if rendering is successfully resumed, false otherwise.
         */
        bool resume_render();
        /**
         * @brief Stops the rendering process.
         * @return True if rendering is successfully stopped, false otherwise.
         */
        bool stop_render();
        /**
         * @brief Updates the rendering process.
         */
        void update_render();
        /**
         * @brief Waits for the rendering process to complete.
         */
        void wait_render();
        /**
         * @brief Retrieves the current rendering status.
         * @return The current rendering status.
         */
        RenderStatus get_render_status();
        /**
         * @brief Retrieves the current rendering method.
         * @return The current rendering method.
         */
        RenderMethod get_render_method();
        /**
         * @brief Dumps debug output to the specified output file path.
         * @param output_file_path The path of the output file to dump the debug output to.
         * @return True if dumping is successful, false otherwise.
         */
        bool dump_debug_output(const std::string& output_file_path);

    private:
        IRenderControlPtr m_render_control = nullptr;
        std::function<void(RenderStatus)> m_at_finish;
    };
}
#define plugin_render extern "C" OPENDCC_API_EXPORT void render_plugin_entry()

OPENDCC_NAMESPACE_CLOSE
