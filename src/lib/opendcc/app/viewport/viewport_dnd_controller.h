/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <memory>
#include "opendcc/app/viewport/viewport_scene_context.h"

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

OPENDCC_NAMESPACE_OPEN
class ViewportView;

class OPENDCC_API ViewportDndController
{
public:
    ViewportDndController(const PXR_NS::TfToken& context_type);
    ~ViewportDndController() = default;

    void set_scene_context(const PXR_NS::TfToken& context_type);
    void on_enter(std::shared_ptr<ViewportView> view, QDragEnterEvent* event);
    void on_move(std::shared_ptr<ViewportView> view, QDragMoveEvent* event);
    void on_drop(std::shared_ptr<ViewportView> view, QDropEvent* event);
    void on_leave(std::shared_ptr<ViewportView> view, QDragLeaveEvent* event);
    void on_view_destroyed(std::shared_ptr<ViewportView> view);

private:
    PXR_NS::TfToken m_scene_context_type;
};
OPENDCC_NAMESPACE_CLOSE
