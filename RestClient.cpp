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

#include "RestClient.h"

RestClient::RestClient(const QString& apiUrl, QObject *parent) :
    QObject(parent),
    _apiUrl(apiUrl),
    _nam(this)
{

}
void RestClient::get (const QString& path, const QVariant& arg, const char* slotName)
{
    QNetworkRequest request(_apiUrl + path);
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    QNetworkReply* reply = _nam.get (request);
    connect (reply, &QNetworkReply::finished, this,  [=] () {
        QString answer = reply->readAll();

        // convert to json
        QJsonParseError parseerror;
        QJsonDocument doc = QJsonDocument::fromJson(answer.toUtf8(), &parseerror);
        if (parseerror.error != QJsonParseError::NoError) {
            emit error ("JSON error : " + parseerror.errorString());
            return;
        }
        QVariantMap map = doc.toVariant().toMap();
        QMetaObject::invokeMethod( parent(), slotName + 1, Q_ARG(QVariantMap, map), Q_ARG(QVariant, arg) );
    });
}
