// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/viewport_dnd_controller.h"
#include "opendcc/app/viewport/viewport_dnd_callback_registry.h"

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

ViewportDndController::ViewportDndController(const TfToken& context_type)
    : m_scene_context_type(context_type)
{
}

void ViewportDndController::set_scene_context(const TfToken& context_type)
{
    m_scene_context_type = context_type;
}

void ViewportDndController::on_enter(std::shared_ptr<ViewportView> view, QDragEnterEvent* event)
{
    for (const auto& callback : ViewportDndCallbackRegistry::get_callbacks(m_scene_context_type))
        callback->on_enter(view, event);
}

void ViewportDndController::on_move(std::shared_ptr<ViewportView> view, QDragMoveEvent* event)
{
    for (const auto& callback : ViewportDndCallbackRegistry::get_callbacks(m_scene_context_type))
        callback->on_move(view, event);
}

void ViewportDndController::on_drop(std::shared_ptr<ViewportView> view, QDropEvent* event)
{
    for (const auto& callback : ViewportDndCallbackRegistry::get_callbacks(m_scene_context_type))
        callback->on_drop(view, event);
}

void ViewportDndController::on_leave(std::shared_ptr<ViewportView> view, QDragLeaveEvent* event)
{
    for (const auto& callback : ViewportDndCallbackRegistry::get_callbacks(m_scene_context_type))
        callback->on_leave(view, event);
}

void ViewportDndController::on_view_destroyed(std::shared_ptr<ViewportView> view)
{
    for (const auto& callback : ViewportDndCallbackRegistry::get_callbacks(m_scene_context_type))
        callback->on_view_destroyed(view);
}
OPENDCC_NAMESPACE_CLOSE