/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <memory>
#include <vector>
#include <functional>
#include <pxr/base/tf/token.h>
#include "opendcc/app/viewport/iviewport_draw_extension.h"

OPENDCC_NAMESPACE_OPEN
class ViewportWidget;
class IViewportUIExtension;
typedef std::shared_ptr<IViewportUIExtension> IViewportUiExtensionPtr;

/** @brief The IViewportUIExtension class allows user to create
 * the ViewportDrawExtensions in each application's Viewport.
 * It provides additional features for
 * Viewport user interface, e.g. brushes.
 */
class IViewportUIExtension
{
public:
    IViewportUIExtension(ViewportWidget* viewport_widget)
        : m_viewport_widget(viewport_widget) {};
    virtual ~IViewportUIExtension() {};
    /**
     * @brief Gets a pointer to the Viewport widget.
     *
     */
    ViewportWidget* get_viewport_widget() { return m_viewport_widget; }
    /**
     * @brief A virtual method which allows to create
     * additional Viewport extensions which are used for drawing.
     *
     * @return An array of custom Viewport draw extensions.
     */
    std::vector<IViewportDrawExtensionPtr> virtual create_draw_extensions() { return std::vector<IViewportDrawExtensionPtr>(); };

private:
    ViewportWidget* m_viewport_widget;
};

class OPENDCC_API ViewportUIExtensionRegistry final
{
public:
    static ViewportUIExtensionRegistry& instance();
    void register_ui_extension(const PXR_NS::TfToken& name, std::function<IViewportUiExtensionPtr(ViewportWidget*)> factory_fn);
    void unregister_ui_extension(const PXR_NS::TfToken& name);

    /**
     * @brief Creates instances of all registered IViewportUIExtension
     * for the specified ViewportWidget.
     *
     * @param viewport_widget A widget for which to create the extensions.
     * @return All registered IViewportUIExtension instances.
     */
    static std::vector<IViewportUiExtensionPtr> create_extensions(ViewportWidget* viewport_widget);

private:
    ViewportUIExtensionRegistry() = default;
    ViewportUIExtensionRegistry(const ViewportUIExtensionRegistry&) = delete;
    ViewportUIExtensionRegistry(ViewportUIExtensionRegistry&&) = delete;
    ViewportUIExtensionRegistry& operator=(const ViewportUIExtensionRegistry&) = delete;
    ViewportUIExtensionRegistry& operator=(ViewportUIExtensionRegistry&&) = delete;

    std::unordered_map<PXR_NS::TfToken, std::function<IViewportUiExtensionPtr(ViewportWidget*)>, PXR_NS::TfToken::HashFunctor> m_registry;
};

OPENDCC_NAMESPACE_CLOSE
