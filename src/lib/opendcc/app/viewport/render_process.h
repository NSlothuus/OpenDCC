/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/app/core/api.h"
#include "opendcc/base/vendor/tiny_process/process.hpp"
#include "opendcc/app/ui/logger/render_catalog.h"
#include "opendcc/render_system/irender.h"
#include <mutex>
#include <future>

OPENDCC_NAMESPACE_OPEN

class OPENDCC_API RenderProcess
{
public:
    RenderProcess(const std::string& cmd, const std::string& catalog, CatalogDataPtr catalog_data);
    ~RenderProcess();

    void start();
    render_system::RenderStatus get_status();
    void wait();
    void stop();

private:
    std::string m_cmd;
    std::string m_catalog;
    CatalogDataPtr m_catalog_data;
    std::mutex m_status_mtx;
    std::condition_variable m_status_var;
    std::unique_ptr<TinyProcessLib::Process> m_render_process;
    render_system::RenderStatus m_status { render_system::RenderStatus::NOT_STARTED };
};

OPENDCC_NAMESPACE_CLOSE
