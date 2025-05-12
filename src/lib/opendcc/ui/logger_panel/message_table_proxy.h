/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include <QSortFilterProxyModel>
#include <QSet>
#include "opendcc/base/logging/logger.h"

OPENDCC_NAMESPACE_OPEN
class MessageTableProxy : public QSortFilterProxyModel
{
    Q_OBJECT;

public:
    MessageTableProxy(QObject* parent = nullptr);

    void set_channels(const QSet<QString>& channels);
    void set_log_level_mask(uint32_t log_level_mask);
    void set_search_query(const QString& query);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

private:
    QSet<QString> m_filter_channels;
    uint32_t m_log_level_filter = 0;
    QString m_search_query = "";
};
OPENDCC_NAMESPACE_CLOSE
