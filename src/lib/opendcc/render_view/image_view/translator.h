/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "opendcc/opendcc.h"
#include "opendcc/render_view/image_view/api.h"

#include <QDir>
#include <QMap>
#include <QString>
#include <QVector>
class QTranslator;

OPENDCC_NAMESPACE_OPEN

class OPENDCC_RENDER_VIEW_API Translator
{
private:
    Translator();

    Translator(const Translator&) = delete;
    Translator(Translator&&) = delete;
    Translator& operator=(const Translator&) = delete;
    Translator& operator=(Translator&&) = delete;

public:
    static Translator& instance();

    const QDir& get_i18n_dir() const;
    const QVector<QString>& get_supported_languages() const;
    QList<QString> get_supported_beauty_languages() const;

    QString from_beauty(const QString& beauty) const;
    QString to_beauty(const QString& language) const;

    bool set_language(const QString& language);

private:
    QDir m_i18n_dir;
    QVector<QString> m_supported_languages;
    QTranslator* m_translator = nullptr;
    QMap<QString, QString> m_enum_name_map;
};

OPENDCC_NAMESPACE_CLOSE
