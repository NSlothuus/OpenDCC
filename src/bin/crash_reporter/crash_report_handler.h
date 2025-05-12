/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/base/defines.h"
#include <crashpad/util/misc/uuid.h>
#include <vector>

#include <sentry.h>
#include "crashpad/client/crashpad_client.h"
#include "crashpad/client/crash_report_database.h"
#include "crashpad/handler/crash_report_upload_thread.h"
#include "crashpad/client/settings.h"

OPENDCC_NAMESPACE_OPEN

class CrashReportHandler
{
public:
    static std::shared_ptr<CrashReportHandler> create(const std::string& db_path, const std::string& dsn);

    ~CrashReportHandler();

    bool has_reports() const;
    crashpad::CrashReportDatabase::Report get_last_report() const;

    const std::vector<crashpad::CrashReportDatabase::Report>& get_reports() const;

    std::string upload_report(const crashpad::UUID& report_uuid, const std::string& username, const std::string& email, const std::string& feedback);

private:
    CrashReportHandler(std::unique_ptr<crashpad::CrashReportDatabase> database, sentry_options_t* options);

    std::unique_ptr<crashpad::CrashReportDatabase> m_database;
    std::unique_ptr<crashpad::CrashReportUploadThread> m_crash_report_upload_thread;
    std::vector<crashpad::CrashReportDatabase::Report> m_reports;
    sentry_options_t* m_options = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
