/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/app/core/api.h"
#include "opendcc/render_system/irender.h"
#include "opendcc/render_system/render_system.h"

OPENDCC_NAMESPACE_OPEN

class UsdRender;

class OPENDCC_API UsdRenderControl : public render_system::IRenderControl
{
public:
    UsdRenderControl(const std::string& scene_context, std::shared_ptr<UsdRender> hydra_render);
    ~UsdRenderControl() override = default;
    UsdRenderControl(const UsdRenderControl&) = delete;
    UsdRenderControl(UsdRenderControl&&) = delete;
    UsdRenderControl& operator=(UsdRenderControl&&) = delete;
    UsdRenderControl& operator=(const UsdRenderControl&) = delete;

    virtual std::string control_type() const override;
    virtual void set_attributes(const render_system::RenderAttributes& attributes) override;
    virtual bool init_render(render_system::RenderMethod type) override;
    virtual bool start_render() override;
    virtual bool pause_render() override;
    virtual bool resume_render() override;
    virtual bool stop_render() override;
    virtual void update_render() override;
    virtual void wait_render() override;
    virtual void set_resolver(const std::string&) override;
    virtual render_system::RenderStatus render_status() override;
    virtual render_system::RenderMethod render_method() const override;
    virtual void finished(std::function<void(render_system::RenderStatus)> cb) override;

private:
    render_system::RenderStatus m_render_status = render_system::RenderStatus::NOT_STARTED;
    render_system::RenderMethod m_render_method = render_system::RenderMethod::NONE;
    std::shared_ptr<UsdRender> m_hydra_render;
    std::string m_scene_context;
    render_system::RenderAttributes m_attributes;
};

OPENDCC_NAMESPACE_CLOSE
