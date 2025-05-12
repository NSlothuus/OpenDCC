/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"

#include "opendcc/app/core/api.h"
#include "opendcc/usd/compositing/layer.h"
#include "opendcc/usd/compositing/compositor.h"

#include <pxr/base/tf/type.h>

#include <memory>

OPENDCC_NAMESPACE_OPEN

class ViewportGLWidget;

//////////////////////////////////////////////////////////////////////////
// IViewportCompositingExtension
//////////////////////////////////////////////////////////////////////////

class IViewportCompositingExtension
{
public:
    OPENDCC_API IViewportCompositingExtension(ViewportGLWidget* widget);
    OPENDCC_API virtual ~IViewportCompositingExtension();

    OPENDCC_API ViewportGLWidget* get_widget();

    virtual LayerPtr create_layer() = 0;

    static CompositorPtr create_compositor(ViewportGLWidget* widget);

private:
    ViewportGLWidget* m_widget;
};
using IViewportCompositingExtensionPtr = std::shared_ptr<IViewportCompositingExtension>;

//////////////////////////////////////////////////////////////////////////
// ViewportCompositingExtensionFactoryBase
//////////////////////////////////////////////////////////////////////////

class ViewportCompositingExtensionFactoryBase : public PXR_NS::TfType::FactoryBase
{
public:
    OPENDCC_API virtual ~ViewportCompositingExtensionFactoryBase();

    virtual IViewportCompositingExtensionPtr create(ViewportGLWidget* widget) const = 0;
};

//////////////////////////////////////////////////////////////////////////
// ViewportCompositingExtensionFactory
//////////////////////////////////////////////////////////////////////////

template <class T>
class ViewportCompositingExtensionFactory : public ViewportCompositingExtensionFactoryBase
{
public:
    virtual ~ViewportCompositingExtensionFactory() {}

    IViewportCompositingExtensionPtr create(ViewportGLWidget* widget) const override { return std::make_shared<T>(widget); }
};

OPENDCC_NAMESPACE_CLOSE
