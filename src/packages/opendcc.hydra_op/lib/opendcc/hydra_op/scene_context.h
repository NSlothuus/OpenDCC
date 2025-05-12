/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/app/viewport/viewport_scene_context.h"
#include "opendcc/hydra_op/scene_index_manager.h"
#include "opendcc/hydra_op/session.h"

OPENDCC_NAMESPACE_OPEN

class HydraOpViewportRenderSettings;
class HydraOpSceneContext final : public ViewportSceneContext
{
public:
    HydraOpSceneContext(const PXR_NS::TfToken& name);
    ~HydraOpSceneContext();
    std::shared_ptr<SceneIndexManager> get_index_manager() const override;

    SelectionList get_selection() const override;
    void resolve_picking(PXR_NS::HdxPickHitVector& pick_hits) override;

    bool use_hydra2() const { return true; }
    virtual SceneDelegateCollection get_delegates() const override;
    virtual std::shared_ptr<HydraRenderSettings> get_render_settings() const override;

private:
    void rebuild_render_settings();

    std::shared_ptr<HydraOpSceneIndexManager> m_si_manager;
    std::shared_ptr<HydraOpViewportRenderSettings> m_render_settings;
    HydraOpSession::Handle m_view_node_changed;
};

OPENDCC_NAMESPACE_CLOSE
