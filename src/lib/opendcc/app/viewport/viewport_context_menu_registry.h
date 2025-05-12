/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/app/viewport/viewport_view.h"
#include <pxr/base/tf/token.h>

class QMenu;
class QWidget;
class QContextMenuEvent;

OPENDCC_NAMESPACE_OPEN

class IViewportToolContext;

class OPENDCC_API ViewportContextMenuRegistry
{
public:
    using CreateContextMenuFn = std::function<QMenu*(QContextMenuEvent*, ViewportViewPtr, QWidget*)>;
    static ViewportContextMenuRegistry& instance();

    bool register_menu(const PXR_NS::TfToken& context_type, CreateContextMenuFn creator);
    bool unregister_menu(const PXR_NS::TfToken& context_type);

    QMenu* create_menu(const PXR_NS::TfToken& context_type, QContextMenuEvent* context_menu_event, ViewportViewPtr viewport_view, QWidget* parent);

private:
    std::unordered_map<PXR_NS::TfToken, CreateContextMenuFn, PXR_NS::TfToken::HashFunctor> m_registry;
};
OPENDCC_NAMESPACE_CLOSE