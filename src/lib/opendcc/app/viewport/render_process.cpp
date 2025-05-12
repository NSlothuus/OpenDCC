// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/app/viewport/render_process.h"
#include "opendcc/app/viewport/usd_render.h"
#include "opendcc/base/utils/process.h"

#include <atomic>
#include <fstream>
#include <iomanip>
#include <functional>
#include <mutex>

OPENDCC_NAMESPACE_OPEN

PXR_NAMESPACE_USING_DIRECTIVE

using namespace render_system;

static void log_process_message(const std::string& catalog, const char* bytes, size_t n)
{
    const auto input = std::string(bytes, n);
    std::vector<std::string> split_list = TfStringSplit(input, "\n");
    for (const auto& item : split_list)
    {
        RenderCatalog::instance().add_msg(catalog, item);
    }
}

RenderProcess::RenderProcess(const std::string& cmd, const std::string& catalog, CatalogDataPtr catalog_data)
    : m_cmd(cmd)
    , m_catalog(catalog)
    , m_catalog_data(catalog_data)
{
}

void RenderProcess::stop()
{
    auto wait_until_process_finalization = false;
    {
        std::lock_guard<std::mutex> lock(m_status_mtx);
        int exit_status;
        if (m_render_process && !m_render_process->try_get_exit_status(exit_status))
        {
            wait_until_process_finalization = true;
            m_status = render_system::RenderStatus::STOPPED;
            m_render_process->kill(true);
        }
    }
    if (wait_until_process_finalization)
    {
        std::unique_lock<std::mutex> lock(m_status_mtx);
        m_status_var.wait(lock);
    }
}

void RenderProcess::wait()
{
    std::unique_lock<std::mutex> lock(m_status_mtx);
    m_status_var.wait(lock, [this] {
        return m_status != RenderStatus::NOT_STARTED && m_status != RenderStatus::IN_PROGRESS && m_status != RenderStatus::RENDERING;
    });
}

render_system::RenderStatus RenderProcess::get_status()
{
    std::lock_guard<std::mutex> lock(m_status_mtx);
    return m_status;
}

void RenderProcess::start()
{
    std::unique_ptr<LiveShareSession> live_share;
    auto t_start = std::time(nullptr);
    auto& render_catalog = RenderCatalog::instance();
    {
        std::lock_guard<std::mutex> lock(m_status_mtx);
        if (m_status != render_system::RenderStatus::NOT_STARTED)
        {
            return;
        }
        m_status = render_system::RenderStatus::IN_PROGRESS;

        const auto log_function = [this](const char* bytes, size_t n) {
            log_process_message(m_catalog, bytes, n);
        };

        OPENDCC_INFO("Start out of process USD render: {}", m_cmd);
        render_catalog.add_msg(m_catalog, "Start out of process render: \n " + m_cmd + "\n");

        t_start = std::time(nullptr);

        m_render_process = std::make_unique<TinyProcessLib::Process>(m_cmd, std::string(), log_function, log_function);
        m_status = RenderStatus::RENDERING;
    }
    // wait
    auto exit_status = m_render_process->get_exit_status();

    const auto t_end = std::time(nullptr);
    const auto t_diff = t_end - t_start;
    const auto tm = *std::localtime(&t_diff);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");

    m_catalog_data->elapsed_time = oss.str();
    render_catalog.update_catalog_info(m_catalog);
    render_catalog.add_msg(m_catalog, "Out of process render finished: exit status " + std::to_string(exit_status) + "\n");

    {
        std::lock_guard<std::mutex> lock(m_status_mtx);
        if (m_status != RenderStatus::STOPPED)
        {
            m_status = exit_status ? render_system::RenderStatus::FAILED : render_system::RenderStatus::FINISHED;
        }
        m_render_process.reset();
    }
    m_status_var.notify_all();
}

RenderProcess::~RenderProcess()
{
    stop();
}

OPENDCC_NAMESPACE_CLOSE
