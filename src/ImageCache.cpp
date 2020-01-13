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

#include "ImageCache.h"

#include <QDir>
#include <QFile>

ImageCache::ImageCache(const QString& baseUrl, const QString& directory, QLoggingCategory& log, bool lock,
                       QStringList* filter, QObject* parent)
    : QObject(parent),
      _baseUrl(baseUrl),
      _directory(directory),
      _filter(filter == nullptr ? QStringList({"*.jpg", "*.png"}) : *filter),
      _log(log),
      _lock(lock ? new QMutex() : nullptr) {
    if (!_directory.endsWith("/")) {
        _directory += "/";
    }
    initialize();
}
ImageCache::~ImageCache() {
    if (_lock != nullptr) {
        delete _lock;
    }
}
QString ImageCache::makeFileName(const QString& path) {
    // QML image does not support encoded file urls
    // return QUrl::toPercentEncoding(path);
    return path;
}
bool ImageCache::get(const QString& path, QString& fileUrl) {
    QString fileName = makeFileName(path);
#if _WIN32
    // file:///c:/temp/x.url
    int idx = _directory.indexOf(':');
    if (idx >= 0) {
        // fileUrl = "file:///" + _directory.left (idx + 1) + _directory.mid(idx + 1) + fileName;
        fileUrl = "file://" + _directory.mid(idx + 1) + fileName;
    } else {
        fileUrl = "file://" + _directory + fileName;
    }
#else
    fileUrl = "file://" + _directory + fileName;
#endif
    {
        QMutexLocker lock(_lock);
        if (_cache.contains(fileName)) {
            return true;
        }
        if (_loading.contains(fileName)) {
            return false;
        }
        _loading.insert(fileName);
    }
    fetch(path, fileName);
    return false;
}

void ImageCache::fetch(const QString& path, const QString& fileName) {
    QNetworkRequest req(_baseUrl + path);
    QNetworkReply*  reply = _nam.get(req);
    //? connect (reply, &QNetworkReply::error, this, &ImageCache::onError);
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError code) { fetchError(code, fileName); });
    connect(reply, &QNetworkReply::finished, this, [=]() {
        QByteArray data = reply->readAll();
        if (data.length() > 0) {
            storeFile(data, fileName);
        } else {
            fetchError(QNetworkReply::NetworkError::ContentConflictError, fileName);
        }
    });
}
void ImageCache::storeFile(const QByteArray& data, const QString& fileName) {
    QFile file(_directory + fileName);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();

    qCDebug(_log) << "fetched file" << fileName;

    QMutexLocker lock(_lock);
    _cache.insert(fileName);
    _loading.remove(fileName);
    emit loaded(fileName);
    if (_loading.count() == 0) {
        emit allLoaded();
    }
}
void ImageCache::fetchError(QNetworkReply::NetworkError code, const QString& fileName) {
    qCWarning(_log) << "Can't fetch" << fileName << " Error: " << code;

    QMutexLocker lock(_lock);
    _loading.remove(fileName);
    emit error("fetch error file " + fileName);
    if (_loading.count() == 0) {
        emit allLoaded();
    }
}
void ImageCache::initialize() {
    QDir        dir(_directory);
    QStringList fileNames = dir.entryList(_filter, QDir::Files);
    for (QStringList::iterator i = fileNames.begin(); i != fileNames.end(); ++i) {
        if (i->startsWith('.')) {
            continue;
        }
        _cache.insert(*i);
    }
}
