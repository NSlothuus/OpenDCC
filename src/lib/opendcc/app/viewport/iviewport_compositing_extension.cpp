// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/iviewport_compositing_extension.h"
#include "opendcc/app/viewport/viewport_gl_widget.h"

#include <pxr/base/tf/type.h>
#include <pxr/base/plug/registry.h>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

//////////////////////////////////////////////////////////////////////////
// IViewportCompositingExtension
//////////////////////////////////////////////////////////////////////////

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<IViewportCompositingExtension>();
}

IViewportCompositingExtension::IViewportCompositingExtension(ViewportGLWidget* widget)
    : m_widget(widget)
{
}

/* virtual */
IViewportCompositingExtension::~IViewportCompositingExtension() {}

ViewportGLWidget* IViewportCompositingExtension::get_widget()
{
    return m_widget;
}

/* static */
CompositorPtr IViewportCompositingExtension::create_compositor(ViewportGLWidget* widget)
{
    std::set<TfType> types;
    PlugRegistry::GetInstance().GetAllDerivedTypes<IViewportCompositingExtension>(&types);

    CompositorPtr compositor = std::make_shared<Compositor>();
    for (const TfType& type : types)
    {
        const auto factory = type.GetFactory<ViewportCompositingExtensionFactoryBase>();
        if (!TF_VERIFY(factory))
        {
            continue;
        }

        const auto extension = factory->create(widget);
        if (!extension)
        {
            continue;
        }

        const auto layer = extension->create_layer();
        if (!layer)
        {
            continue;
        }

        compositor->add_layer(layer);
    }

    return compositor;
}

//////////////////////////////////////////////////////////////////////////
// ViewportCompositingExtensionFactoryBase
//////////////////////////////////////////////////////////////////////////

ViewportCompositingExtensionFactoryBase::~ViewportCompositingExtensionFactoryBase() {}

OPENDCC_NAMESPACE_CLOSE
