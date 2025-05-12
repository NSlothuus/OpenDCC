// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "crash_report_handler.h"

extern "C"
{
#include <sentry_core.h>
#include <sentry_transport.h>
#include <sentry_envelope.h>
#include <sentry_database.h>
}

#include <filesystem>
#include <iostream>

OPENDCC_NAMESPACE_OPEN

CrashReportHandler::CrashReportHandler(std::unique_ptr<crashpad::CrashReportDatabase> database, sentry_options_t* options)
    : m_database(std::move(database))
    , m_options(options)
{
    m_database->GetCompletedReports(&m_reports);

    const auto proxy = "";
    auto dsn = sentry__dsn_new(sentry_options_get_dsn(options));
    auto url_cstr = sentry__dsn_get_minidump_url(dsn, sentry_options_get_user_agent(options));
    auto url = std::string(url_cstr);
    sentry_free(url_cstr);

    sentry__dsn_decref(dsn);

    crashpad::CrashReportUploadThread::Options thread_opts;
    thread_opts.rate_limit = false;
    thread_opts.watch_pending_reports = false;
    thread_opts.upload_gzip = true;
    thread_opts.identify_client_via_url = true;

    m_crash_report_upload_thread = std::make_unique<crashpad::CrashReportUploadThread>(m_database.get(), url, proxy, thread_opts, nullptr);
    m_crash_report_upload_thread->Start();
}

std::shared_ptr<CrashReportHandler> CrashReportHandler::create(const std::string& database_path, const std::string& dsn)
{
    std::filesystem::path db_fs_path(database_path);

#ifdef OPENDCC_OS_WINDOWS
    const auto db_path = base::FilePath(db_fs_path.wstring());
#else
    const auto db_path = base::FilePath(db_fs_path.string());
#endif
    auto db = crashpad::CrashReportDatabase::InitializeWithoutCreating(db_path);
    if (!db)
    {
        return nullptr;
    }

    auto options = sentry_options_new();
    sentry_options_set_dsn(options, dsn.c_str());
    const auto tmp_dir_path = db_fs_path.string();
    sentry_options_set_database_path(options, tmp_dir_path.c_str());

    return std::shared_ptr<CrashReportHandler>(new CrashReportHandler(std::move(db), std::move(options)));
}

CrashReportHandler::~CrashReportHandler()
{
    sentry_options_free(m_options);
}

bool CrashReportHandler::has_reports() const
{
    return get_last_report().uuid != crashpad::UUID();
}

crashpad::CrashReportDatabase::Report CrashReportHandler::get_last_report() const
{
    time_t latest = 0;
    size_t ind = 0;
    for (int i = 0; i < m_reports.size(); ++i)
    {
        if (m_reports[i].creation_time > latest && !m_reports[i].uploaded)
        {
            latest = m_reports[i].creation_time;
            ind = i;
        }
    }

    if (latest != 0)
    {
        return m_reports[ind];
    }
    return crashpad::CrashReportDatabase::Report();
}

const std::vector<crashpad::CrashReportDatabase::Report>& CrashReportHandler::get_reports() const
{
    return m_reports;
}

std::string CrashReportHandler::upload_report(const crashpad::UUID& report_uuid, const std::string& username, const std::string& email,
                                              const std::string& feedback)
{
    const auto upload_result = m_database->RequestUpload(report_uuid);
    if (upload_result != crashpad::CrashReportDatabase::kNoError)
    {
        std::cerr << "Failed to request update: error " << upload_result << '\n';
        return "";
    }

    m_crash_report_upload_thread->Start();
    m_crash_report_upload_thread->ReportPending(report_uuid);
    m_crash_report_upload_thread->Stop();

    m_reports.clear();
    m_database->GetCompletedReports(&m_reports);

    std::string event_id;

    for (const auto& r : m_reports)
    {
        if (r.uuid == report_uuid)
        {
            event_id = r.id;
            break;
        }
    }

    if (event_id.empty())
    {
        return event_id;
    }

    auto uuid = sentry_uuid_from_string(event_id.c_str());

    // usage of sentry' private API in order to avoid full sentry initialization and pruning crashpad database
    auto feedback_entry = sentry_value_new_user_feedback(&uuid, username.c_str(), email.c_str(), feedback.c_str());

    if (auto envelope = sentry__envelope_new())
    {
        auto result = sentry__envelope_add_user_feedback(envelope, feedback_entry);
        if (!result)
        {
            sentry_envelope_free(envelope);
            sentry_value_decref(feedback_entry);
            std::cerr << "Failed to create user feedback\n";
            return "";
        }
        else
        {
            auto transport = sentry__transport_new_default();
            sentry__transport_startup(transport, m_options);
            sentry__transport_send_envelope(transport, envelope);
            sentry__transport_flush(transport, 10'000);
            sentry__transport_shutdown(transport, 10'000);
            sentry_transport_free(transport);
        }
    }

    sentry_value_decref(feedback_entry);
    return event_id;
}

OPENDCC_NAMESPACE_CLOSE
