/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <OpenImageIO/oiioversion.h>
#include "opendcc/opendcc.h"
#include "opendcc/usd_editor/usd_node_editor/api.h"
#include "opendcc/ui/node_editor/thumbnail_cache.h"
#include <QCache>
#include <mutex>

OIIO_NAMESPACE_BEGIN
class ImageCache;
OIIO_NAMESPACE_END

OPENDCC_NAMESPACE_OPEN

class OPENDCC_USD_NODE_EDITOR_API OIIOThumbnailCache : public ThumbnailCache
{
    Q_OBJECT
public:
    OIIOThumbnailCache(QObject* parent = nullptr);
    ~OIIOThumbnailCache() override;

    bool has_image(const QString& path) const override;
    QImage* read_image(const QString& path) override;
    void read_image_async(const QString& path) override;
    void insert_image(const QString& path, QImage* image);

private:
    QCache<QString, QImage> m_cache;
    mutable std::mutex m_mutex;
    OIIO::ImageCache* m_oiio_cache = nullptr;
};

OPENDCC_NAMESPACE_CLOSE
