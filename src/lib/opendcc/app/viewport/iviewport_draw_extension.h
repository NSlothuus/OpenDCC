/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <memory>
#include <pxr/base/gf/frustum.h>

OPENDCC_NAMESPACE_OPEN

class ViewportUiDrawManager;
class IViewportDrawExtension;
typedef std::shared_ptr<IViewportDrawExtension> IViewportDrawExtensionPtr;

/**
 * @brief The IViewportDrawExtension is a base class which
 * provides drawing simple geometry outside of the ViewportToolContext.
 * Using the ``draw`` method is passed to the virtual ``ViewportUiDrawManager``.
 *
 */
class IViewportDrawExtension
{
public:
    IViewportDrawExtension() {};
    virtual ~IViewportDrawExtension() {};
    /**
     * @brief A virtual method which allows user to draw directly in the Viewport.
     * Calling of this method takes place after execution of hydra engine and
     * calling ``draw`` for the current ViewportToolContext.
     *
     * @note Multiple IViewportDrawExtension instances are called in the undefined order.
     *
     * @param draw_manager The UI draw manager which is to be used for simple geometry drawing.
     * @param frustum The Frustum containing the information about camera of the Viewport in which to draw:
     *
     *       - The position of the viewpoint.
     *       - The rotation applied to the default view frame,
     *          which searches along the -z axis with the +y axis as the "up" direction.
     *       - The 2D window on the reference plane that defines
     *          the left, right, top, and bottom planes of the viewing frustum, as described below.
     *       - The distances to the near and far planes.
     *       - The projection type.
     *       - The view distance.
     *
     * @param width The width of the Viewport in which to draw.
     * @param height The height of the Viewport in which to draw.
     */
    virtual void draw(ViewportUiDrawManager* draw_manager, const PXR_NS::GfFrustum& frustum, int width, int height) = 0;
};

OPENDCC_NAMESPACE_CLOSE