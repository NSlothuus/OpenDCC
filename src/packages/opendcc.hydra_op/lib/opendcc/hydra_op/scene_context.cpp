// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "scene_context.h"
#include "opendcc/hydra_op/session.h"
#include "opendcc/hydra_op/viewport_render_settings.h"

PXR_NAMESPACE_USING_DIRECTIVE

OPENDCC_NAMESPACE_OPEN

HydraOpSceneContext::HydraOpSceneContext(const PXR_NS::TfToken& name)
    : ViewportSceneContext(name)
{
    m_si_manager = std::make_shared<HydraOpSceneIndexManager>();
    m_render_settings = HydraOpViewportRenderSettings::create(m_si_manager->get_terminal_index());
    m_view_node_changed =
        HydraOpSession::instance().register_event_handler(HydraOpSession::EventType::ViewNodeChanged, [this] { rebuild_render_settings(); });
}

HydraOpSceneContext::~HydraOpSceneContext()
{
    HydraOpSession::instance().unregister_event_handler(HydraOpSession::EventType::ViewNodeChanged, m_view_node_changed);
}

std::shared_ptr<SceneIndexManager> HydraOpSceneContext::get_index_manager() const
{
    return m_si_manager;
}

SelectionList HydraOpSceneContext::get_selection() const
{
    return HydraOpSession::instance().get_selection();
}

void HydraOpSceneContext::resolve_picking(PXR_NS::HdxPickHitVector& pick_hits)
{
    // nothing for now
}

ViewportSceneContext::SceneDelegateCollection HydraOpSceneContext::get_delegates() const
{
    return {};
}

std::shared_ptr<HydraRenderSettings> HydraOpSceneContext::get_render_settings() const
{
    return m_render_settings;
}

void HydraOpSceneContext::rebuild_render_settings()
{
    auto new_settings = HydraOpViewportRenderSettings::create(m_si_manager->get_terminal_index());

    if (!m_render_settings && !new_settings)
    {
        return;
    }
    m_render_settings.swap(new_settings);
    dispatch(EventType::DirtyRenderSettings);
}

OPENDCC_NAMESPACE_CLOSE
