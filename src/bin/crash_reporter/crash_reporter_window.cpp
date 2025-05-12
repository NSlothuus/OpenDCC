// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "crash_reporter_window.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>

OPENDCC_NAMESPACE_OPEN

CrashReporterWindow::CrashReporterWindow(std::shared_ptr<CrashReportHandler> crash_report_handler)
    : QMainWindow()
    , m_crash_reporter_handler(crash_report_handler)
{
    setWindowTitle("Crash Reporter");
    if (!m_crash_reporter_handler || !m_crash_reporter_handler->has_reports())
    {
        setCentralWidget(new QLabel("No crashes found."));
        return;
    }

    auto main_layout = new QVBoxLayout;
    main_layout->addWidget(
        new QLabel("A crash has occurred during the work of program. Please, help improve our software and send the crash report."));

    auto form = new QFormLayout;
    m_name_le = new QLineEdit;
    m_name_le->setPlaceholderText("Your name (optional)");
    m_email_le = new QLineEdit;
    m_email_le->setPlaceholderText("user@company.com (optional)");
    m_feedback_te = new QTextEdit;
    m_feedback_te->setPlaceholderText("Tell us what happened. Describe your actions that led to the crash or provide any feedback.");

    form->addRow("Name", m_name_le);
    form->addRow("Email", m_email_le);
    form->addRow("Feedback", m_feedback_te);
    main_layout->addLayout(form);

    auto send_btn = new QPushButton("Send Report");
    connect(send_btn, &QPushButton::clicked, this, &CrashReporterWindow::on_send_report);

    main_layout->addWidget(send_btn);

    auto central_widget = new QWidget;
    central_widget->setLayout(main_layout);
    setCentralWidget(central_widget);
}

CrashReporterWindow::~CrashReporterWindow() {}

void CrashReporterWindow::on_send_report()
{
    auto event_id = m_crash_reporter_handler->upload_report(m_crash_reporter_handler->get_last_report().uuid, m_name_le->text().toStdString(),
                                                            m_email_le->text().toStdString(), m_feedback_te->toPlainText().toStdString());

    if (event_id.empty())
    {
        QMessageBox::warning(this, "Crash Reporting", "Failed to send the crash report.");
    }
    else
    {
        QMessageBox::information(this, "Success", "The crash report has been sent. Thank you.");
        close();
    }
}

OPENDCC_NAMESPACE_CLOSE
