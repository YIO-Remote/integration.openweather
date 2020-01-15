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

#pragma once

#include <QLoggingCategory>
#include <QObject>
#include <QTimer>

#include "ImageCache.h"
#include "WeatherModel.h"
#include "yio-interface/entities/entitiesinterface.h"
#include "yio-interface/entities/weatherinterface.h"
#include "yio-interface/notificationsinterface.h"
#include "yio-interface/plugininterface.h"
#include "yio-plugin/integration.h"

class OpenWeatherPlugin : public PluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "YIO.PluginInterface" FILE "openweather.json")
    Q_INTERFACES(PluginInterface)

 public:
    explicit OpenWeatherPlugin(QObject* parent = nullptr);
    ~OpenWeatherPlugin() override {}

    void create(const QVariantMap& config, QObject* entities, QObject* notifications, QObject* api,
                QObject* configObj) override;
    void setLogEnabled(QtMsgType msgType, bool enable) override { _log.setEnabled(msgType, enable); }

 private:
    QLoggingCategory _log;
};

struct OpenWeatherModel {
    OpenWeatherModel();
    int     date;
    QString description;
    QString imageurl;
    int     temp;
    int     tempmin;
    int     tempmax;
    int     rain;
    int     snow;
    int     wind;
    int     humidity;

    int         day() const { return date / 86400; }
    int         hour() const { return (date % 86400) / 3600; }
    WeatherItem toItem(const QString& units, const QString& iconUrl, bool curren);
    void        fromCurrent(const QVariantMap& current);
    void        init(const OpenWeatherModel& model);
    void        add(const OpenWeatherModel& model);

    static QList<OpenWeatherModel> fromForecast(const QVariantMap& forecast);
    static void toDayForecast(QList<OpenWeatherModel>& perDay, const QList<OpenWeatherModel>& _3h);  // NOLINT
};

class OpenWeather : public Integration {
    Q_OBJECT

 public:
    explicit OpenWeather(const QString&    cacheDirectory,
                         QLoggingCategory& log,  // NOLINT we need a non-const reference
                         QObject*          parent = nullptr);
    ~OpenWeather() override;

    Q_INVOKABLE void setup(const QVariantMap& config, QObject* entities, QObject* notifications, QObject* api,
                           QObject* configObj);
    void             connect() override;
    void             disconnect() override;
    void sendCommand(const QString& type, const QString& entity_id, int command, const QVariant& param) override;
    void leaveStandby() override;

 public slots:  // NOLINT open issue: https://github.com/cpplint/cpplint/pull/99
    void onAllImagesLoaded();

 private:
    struct WeatherContext {
        WeatherContext() : index(0), entity(nullptr) {}
        WeatherContext(int index, EntityInterface* entity) : index(index), entity(entity) {}
        int                     index;
        EntityInterface*        entity;
        OpenWeatherModel        current;
        QList<OpenWeatherModel> forecast;
        QList<WeatherItem>      forecastWaitForImages;
    };
    void jsonError(const QString& error);
    void onReplyCurrent(WeatherContext* context, QVariantMap& result);   // NOLINT
    void onReplyForecast(WeatherContext* context, QVariantMap& result);  // NOLINT

    bool applyImageCache(WeatherItem& item, OpenWeatherModel& weatherModel);  // NOLINT
    void getCurrent(WeatherContext* context);
    void getForecast(WeatherContext* context);
    void getAll();

    QLoggingCategory&       _log;
    int                     _cycleHours;
    QString                 _apiUrl;
    QString                 _iconUrl;
    QString                 _key;
    QString                 _language;
    QString                 _units;
    QList<WeatherContext>   _contexts;
    WeatherModel            _model;
    NotificationsInterface* _notifications;
    ImageCache              _imageCache;
    QDateTime               _nextRequest;
    QTimer                  _requestTimer;
    QNetworkAccessManager   _nam;
};
