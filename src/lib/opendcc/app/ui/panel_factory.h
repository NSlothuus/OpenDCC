/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <string>
#include <functional>
#include <unordered_map>

class QWidget;
OPENDCC_NAMESPACE_OPEN

/**
 * @brief A singleton class which allows to register
 * and create custom widgets in the application.
 *
 */
class OPENDCC_API PanelFactory
{
public:
    /**
     * @brief A callback function which creates a custom user widget.
     *
     * The widget ownership is passed to the caller.
     */
    typedef std::function<QWidget*()> PanelFactoryWidgetCallback;

    /**
     * @brief The panel widget initial settings which are used on creation.
     *
     */
    struct Entry
    {
        /**
         * @brief The callback function for widget creation.
         *
         */
        PanelFactoryWidgetCallback callback_fn;
        /**
         * @brief Widget name.
         *
         */
        std::string label;
        /**
         * @brief Widget type.
         *
         */
        std::string type;
        /**
         * @brief Widget group.
         *
         */
        std::string group;
        /**
         * @brief Defines whether more than one of object instances can exist.
         * True if only one object instance can exist, false otherwise.
         */
        bool singleton = false;
        /**
         * @brief Widget icon.
         *
         */
        std::string icon;
    };

    /**
     * @brief Returns a PanelFactory instance.
     *
     */
    static PanelFactory& instance();

    /**
     * @brief Creates a panel widget of the specified type.
     *
     * @param type The widget type to be created.
     * @return A panel widget.
     */
    QWidget* create_panel_widget(const std::string& type);
    /**
     * @brief Registers a new panel.
     *
     * @param type The widget type to be created.
     * @param callback A widget creation callback.
     * @param label The widget name.
     * @param singleton The bool value which defines whether more than one of object instances can exist.
     * True if only one object instance can exist.
     * False if more than one object instances can exist.
     * @param icon The widget icon.
     * @param group The widget group.
     * @return true if the widget was registered, false otherwise.
     */
    bool register_panel(const std::string& type, PanelFactoryWidgetCallback callback, const std::string& label = std::string(),
                        bool singleton = false, const std::string& icon = std::string(), const std::string& group = std::string());
    /**
     * @brief Unregisters a widget of the specified type.
     *
     * @param type The widget type to be unregistered.
     * @return true if the panel was unregistered, false otherwise
     */
    bool unregister_panel(const std::string& type);
    /**
     * @brief Returns the widget name.
     *
     * @param type The widget type to return.
     */
    std::string get_panel_title(const std::string& type);
    /**
     * @brief Returns a collection of the registered widgets.
     *
     */
    const std::unordered_map<std::string, Entry>& get_registry() const;
    /**
     * @brief Creates a registered panel widget
     * object using Entry's widget creation function.
     *
     * @param entry The descriptor of the widget to create.
     * @return The QWidget or nullptr if failed.
     *
     * @note The widget ownership is passed to the caller.
     *
     */
    QWidget* create_panel_widget(const Entry& entry);

private:
    PanelFactory() = default;
    std::unordered_map<std::string, Entry> m_registry_map;
};

OPENDCC_NAMESPACE_CLOSE
