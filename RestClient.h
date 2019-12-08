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

#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>

class RestClient : public QObject
{
    Q_OBJECT
public:
    explicit RestClient(const QString& apiUrl, QObject *parent = nullptr);

    void setApiUrl (const QString& apiUrl)
    {
        _apiUrl = apiUrl;
    }

    void get (const QString& path, const QVariant& arg, const char* slotName);

signals:
    void error (const QString& error);

public slots:

private:
    QString                 _apiUrl;
    QNetworkAccessManager   _nam;
};

#endif // RESTCLIENT_H
