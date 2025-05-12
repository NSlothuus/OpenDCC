/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/viewport/viewport_dnd_callback.h"
#include "opendcc/app/viewport/viewport_scene_context.h"

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API ViewportDndCallbackRegistry
{
public:
    using ViewportDndCallbackVector = std::vector<ViewportDndCallbackPtr>;

    static bool register_callback(const PXR_NS::TfToken& context_type, ViewportDndCallbackPtr callback);
    static bool unregister_callback(const PXR_NS::TfToken& context_type, ViewportDndCallbackPtr callback);

    static const ViewportDndCallbackVector& get_callbacks(const PXR_NS::TfToken& context_type);

private:
    ViewportDndCallbackRegistry() = default;
    static ViewportDndCallbackRegistry& instance();

    using PerContextRegistry = std::unordered_map<PXR_NS::TfToken, ViewportDndCallbackVector, PXR_NS::TfToken::HashFunctor>;
    PerContextRegistry m_registry;
};

OPENDCC_NAMESPACE_CLOSE
