// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#include "opendcc/ui/logger_panel/message_table_proxy.h"
#include "opendcc/ui/logger_panel/logger_widget.h"

OPENDCC_NAMESPACE_OPEN

MessageTableProxy::MessageTableProxy(QObject* parent /* = nullptr */)
    : QSortFilterProxyModel(parent)
{
    m_log_level_filter = LoggerWidget::LogLevelFlags::All;
}

bool MessageTableProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto& model = qobject_cast<MessageModel*>(sourceModel());
    Q_ASSERT(model != nullptr);
    const auto& message = model->message_at(source_row);

    return m_filter_channels.contains(message.channel) && (LoggerWidget::log_level_to_flag(message.log_level) & m_log_level_filter) &&
           message.message.contains(m_search_query);
}

void MessageTableProxy::set_search_query(const QString& query)
{
    m_search_query = query;
    invalidateFilter();
}

void MessageTableProxy::set_log_level_mask(uint32_t log_level_mask)
{
    m_log_level_filter = log_level_mask;
    invalidateFilter();
}

void MessageTableProxy::set_channels(const QSet<QString>& channels)
{
    m_filter_channels = channels;
    invalidateFilter();
}

OPENDCC_NAMESPACE_CLOSE
