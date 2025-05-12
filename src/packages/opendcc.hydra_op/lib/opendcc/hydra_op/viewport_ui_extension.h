/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/hydra_op/session.h"
#include "opendcc/app/viewport/iviewport_draw_extension.h"
#include "opendcc/app/viewport/iviewport_ui_extension.h"

OPENDCC_NAMESPACE_OPEN

class ViewportWidget;

class HydraOpViewportUIExtension : public IViewportUIExtension
{
public:
    HydraOpViewportUIExtension(ViewportWidget* viewport_widget);
    ~HydraOpViewportUIExtension() final;

    std::vector<IViewportDrawExtensionPtr> create_draw_extensions() override;

private:
    HydraOpSession::Handle m_selection_changed_cid;
    HydraOpSession::Handle m_view_node_changed_cid;
};

OPENDCC_NAMESPACE_CLOSE
