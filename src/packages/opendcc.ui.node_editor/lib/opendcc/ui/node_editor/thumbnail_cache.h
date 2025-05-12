/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/ui/node_editor/api.h"
#include <QObject>

class QImage;
class QString;

OPENDCC_NAMESPACE_OPEN

class OPENDCC_UI_NODE_EDITOR_API ThumbnailCache : public QObject
{
    Q_OBJECT
public:
    ThumbnailCache(QObject* parent = nullptr);
    virtual ~ThumbnailCache() {};

    virtual bool has_image(const QString& path) const = 0;
    virtual QImage* read_image(const QString& path) = 0;
    virtual void read_image_async(const QString& path) = 0;
Q_SIGNALS:
    void image_read(const QString& path);
};

OPENDCC_NAMESPACE_CLOSE
