// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_dnd_callback_registry.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

ViewportDndCallbackRegistry& ViewportDndCallbackRegistry::instance()
{
    static ViewportDndCallbackRegistry instance;
    return instance;
}

bool ViewportDndCallbackRegistry::register_callback(const TfToken& context_type, ViewportDndCallbackPtr callback)
{
    instance().m_registry[context_type].push_back(callback);
    return true;
}

bool ViewportDndCallbackRegistry::unregister_callback(const TfToken& context_type, ViewportDndCallbackPtr callback)
{
    auto& controllers = instance().m_registry[context_type];
    controllers.erase(std::remove(controllers.begin(), controllers.end(), callback), controllers.end());
    return true;
}

const ViewportDndCallbackRegistry::ViewportDndCallbackVector& ViewportDndCallbackRegistry::get_callbacks(const TfToken& context_type)
{
    return instance().m_registry[context_type];
}

OPENDCC_NAMESPACE_CLOSE