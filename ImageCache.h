/******************************************************************************
 *
 * Copyright (C) 2019 Christian Riedl <ric@rts.co.at>
 *
 * This file is part of the YIO-Remote software project.
 *
 * YIO-Remote software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YIO-Remote software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YIO-Remote software. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *****************************************************************************/

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLoggingCategory>
#include <QMutex>

class ImageCache : public QObject
{
    Q_OBJECT
public:
    explicit ImageCache(const QString& baseUrl, const QString& directory, QLoggingCategory& log, bool lock = true, QStringList* filter = nullptr, QObject *parent = nullptr);
    ~ImageCache();
    bool get (const QString& path, QString& fileUrl);

signals:
    void allLoaded          ();
    void loaded             (const QString& fileName);
    void error              (const QString& message);

private:
    QString makeFileName    (const QString& path);
    void    fetch           (const QString& path, const QString& fileName);
    void    storeFile       (const QByteArray& data, const QString& fileName);
    void    fetchError      (QNetworkReply::NetworkError error, const QString& fileName);
    void    initialize      ();

    QString                 _baseUrl;
    QString                 _directory;
    QStringList             _filter;
    QLoggingCategory&       _log;
    QMutex*                 _lock;
    QSet<QString>           _cache;
    QSet<QString>           _loading;
    QNetworkAccessManager   _nam;
};

#endif // IMAGECACHE_H
