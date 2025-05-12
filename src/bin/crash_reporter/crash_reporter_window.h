/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "opendcc/opendcc.h"
#include "crash_report_handler.h"

#include <QLineEdit>
#include <QTextEdit>
#include <QMainWindow>

OPENDCC_NAMESPACE_OPEN

class CrashReporterWindow : public QMainWindow
{
public:
    CrashReporterWindow(std::shared_ptr<CrashReportHandler> crash_report_handler);
    ~CrashReporterWindow();

private:
    void on_send_report();
    QLineEdit* m_name_le = nullptr;
    QLineEdit* m_email_le = nullptr;
    QTextEdit* m_feedback_te = nullptr;

    std::shared_ptr<CrashReportHandler> m_crash_reporter_handler;
};

OPENDCC_NAMESPACE_CLOSE
