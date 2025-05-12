// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/usd/render/render_app_controller.h"

OPENDCC_NAMESPACE_OPEN
PXR_NAMESPACE_USING_DIRECTIVE

RenderAppControllerFactory& RenderAppControllerFactory::get_instance()
{
    static RenderAppControllerFactory instance;
    return instance;
}

void RenderAppControllerFactory::register_app_controller(const PXR_NS::TfToken& render_type, const FactoryFn& factory_fn)
{
    auto& ins = get_instance();
    if (ins.m_registry.find(render_type) != ins.m_registry.end())
    {
        return;
    }
    ins.m_registry[render_type] = factory_fn;
}

void RenderAppControllerFactory::unregister_app_controller(const PXR_NS::TfToken& render_type)
{
    get_instance().m_registry.erase(render_type);
}

std::unique_ptr<RenderAppController> RenderAppControllerFactory::make_app_controller(const PXR_NS::TfToken& render_type)
{
    auto& ins = get_instance();
    auto it = ins.m_registry.find(render_type);
    if (it != ins.m_registry.end())
    {
        return it->second();
    }
    return nullptr;
}

RenderAppControllerFactory::RenderAppControllerFactory()
{
    m_registry[TfToken("USD")] = [] {
        return std::make_unique<UsdRenderAppController>();
    };
}

int UsdRenderAppController::process_args(const CLI::App& app)
{
    return 0;
}

OPENDCC_NAMESPACE_CLOSE
