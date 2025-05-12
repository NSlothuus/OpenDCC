// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/render_system/render_system.h"
#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

OPENDCC_NAMESPACE_OPEN

namespace render_system
{
    RenderSystem& RenderSystem::instance()
    {
        static RenderSystem inst;
        return inst;
    }

    bool RenderSystem::load_plugin(const std::string& path)
    {
#ifdef WIN32
        auto handle = LoadLibrary(path.c_str());
        if (!handle)
            return 0;

        typedef void (*Entry)();

        if (Entry entry = (Entry)GetProcAddress(handle, "render_plugin_entry"))
        {
            entry();
            return true;
        }
        else
            return false;
#else
        auto handle = dlopen(path.c_str(), RTLD_NOW);
        if (!handle)
            return 0;

        typedef void (*Entry)();
        if (Entry entry = (Entry)dlsym(handle, "render_plugin_entry"))
        {
            entry();
            return true;
        }
        else
            return false;
#endif
    }

    bool RenderSystem::set_render_control(IRenderControlPtr control)
    {
        if (!control)
            return false;
        m_render_control = control;
        m_render_control->finished([this](RenderStatus rs) {
            if (m_at_finish)
                m_at_finish(rs);
        });
        return true;
    }

    bool RenderSystem::set_render_control(const std::string& control_name)
    {
        const auto& controls = RenderControlHub::instance().get_controls();
        auto it = controls.find(control_name);
        if (it == controls.end())
        {
            return false;
        }
        return set_render_control(it->second);
    }

    void RenderSystem::finished(std::function<void(RenderStatus)> cb)
    {
        m_at_finish = cb;
    }

    bool RenderSystem::init_render(RenderMethod type)
    {
        if (!m_render_control)
            return false;
        return m_render_control->init_render(type);
    }

    bool RenderSystem::start_render()
    {
        if (!m_render_control)
            return false;
        return m_render_control->start_render();
    }

    bool RenderSystem::pause_render()
    {
        if (!m_render_control)
            return false;
        return m_render_control->pause_render();
    }

    bool RenderSystem::resume_render()
    {
        if (!m_render_control)
            return false;
        return m_render_control->resume_render();
    }

    bool RenderSystem::stop_render()
    {
        if (!m_render_control)
            return false;
        return m_render_control->stop_render();
    }

    void RenderSystem::update_render()
    {
        if (!m_render_control)
            return;
        m_render_control->update_render();
    }

    void RenderSystem::wait_render()
    {
        if (!m_render_control)
            return;
        auto status = m_render_control->render_status();
        if (status == RenderStatus::FAILED || status == RenderStatus::FINISHED || status == RenderStatus::STOPPED)
            return;

        m_render_control->wait_render();
    }

    RenderStatus RenderSystem::get_render_status()
    {
        if (!m_render_control)
            return RenderStatus::NOT_STARTED;
        return m_render_control->render_status();
    }

    render_system::RenderMethod RenderSystem::get_render_method()
    {
        if (!m_render_control)
            return RenderMethod::NONE;
        return m_render_control->render_method();
    }

    bool RenderSystem::dump_debug_output(const std::string& output_file_path)
    {
        if (!m_render_control)
            return false;
        return m_render_control->dump(output_file_path);
    }

    const std::unordered_map<std::string, IRenderControlPtr>& RenderControlHub::get_controls() const
    {
        return m_hub;
    }

    bool RenderControlHub::add_render_control(IRenderControlPtr control)
    {
        auto it = m_hub.find(control->control_type());
        if (it != m_hub.end())
            return false;
        m_hub[control->control_type()] = control;
        return true;
    }

    OPENDCC_NAMESPACE::render_system::RenderControlHub& RenderControlHub::instance()
    {
        static RenderControlHub inst;
        return inst;
    }

}

OPENDCC_NAMESPACE_CLOSE
