/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include <memory>

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

OPENDCC_NAMESPACE_OPEN

class ViewportView;

class OPENDCC_API ViewportDndCallback
{
public:
    ViewportDndCallback() = default;
    virtual ~ViewportDndCallback() = default;

    virtual void on_enter(std::shared_ptr<ViewportView> view, QDragEnterEvent* event) {};
    virtual void on_move(std::shared_ptr<ViewportView> view, QDragMoveEvent* event) {};
    virtual void on_drop(std::shared_ptr<ViewportView> view, QDropEvent* event) {};
    virtual void on_leave(std::shared_ptr<ViewportView> view, QDragLeaveEvent* event) {};
    virtual void on_view_destroyed(std::shared_ptr<ViewportView> view) {};
};

using ViewportDndCallbackPtr = std::shared_ptr<ViewportDndCallback>;

OPENDCC_NAMESPACE_CLOSE
