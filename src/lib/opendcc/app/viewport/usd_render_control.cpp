// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/usd_render_control.h"
#include "opendcc/app/viewport/usd_render.h"
#include "opendcc/render_system/render_factory.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

using namespace render_system;

UsdRenderControl::UsdRenderControl(const std::string& scene_context, std::shared_ptr<UsdRender> hydra_render)
    : m_scene_context(scene_context)
    , m_hydra_render(hydra_render)
{
    TF_AXIOM(m_hydra_render);
}

std::string UsdRenderControl::control_type() const
{
    return m_scene_context;
}

void UsdRenderControl::finished(std::function<void(RenderStatus)> cb) {}

render_system::RenderMethod UsdRenderControl::render_method() const
{
    return m_render_method;
}

render_system::RenderStatus UsdRenderControl::render_status()
{
    return m_hydra_render->render_status();
}

void UsdRenderControl::set_resolver(const std::string&) {}

void UsdRenderControl::wait_render()
{
    m_hydra_render->wait_render();
}

void UsdRenderControl::update_render() {}

bool UsdRenderControl::stop_render()
{
    return m_hydra_render->stop_render();
}

bool UsdRenderControl::resume_render()
{
    OPENDCC_ERROR("Unable to resume render. Resume is not supported.");
    return false;
}

bool UsdRenderControl::pause_render()
{
    OPENDCC_ERROR("Unable to pause render. Pause is not supported.");
    return false;
}

bool UsdRenderControl::start_render()
{
    return m_hydra_render->start_render();
}

bool UsdRenderControl::init_render(RenderMethod type)
{
    m_hydra_render->set_attributes(m_attributes);
    return m_hydra_render->init_render(m_render_method = type);
}

void UsdRenderControl::set_attributes(const RenderAttributes& attributes)
{
    for (const auto& it : attributes)
        m_attributes[it.first] = it.second;
}

OPENDCC_NAMESPACE_CLOSE
