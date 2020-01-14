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

#include "OpenWeather.h"

#include <QDir>
#include <QJsonDocument>

#include "yio-interface/configinterface.h"
#include "yio-interface/entities/weatherinterface.h"

IntegrationInterface::~IntegrationInterface() {}

OpenWeatherPlugin::OpenWeatherPlugin(QObject* parent) : _log("openweather") { Q_UNUSED(parent) }

void OpenWeatherPlugin::create(const QVariantMap& config, QObject* entities, QObject* notifications, QObject* api,
                               QObject* configObj) {
    QMap<QObject*, QVariant> returnData;
    QVariantList             data;

    ConfigInterface* configInterface = qobject_cast<ConfigInterface*>(configObj);
    QString          cachePath = configInterface->getContextProperty("configPath").toString() + "/openweather";
    if (!QDir(cachePath).exists()) {
        QDir().mkdir(cachePath);
    }

    for (QVariantMap::const_iterator iter = config.begin(); iter != config.end(); ++iter) {
        if (iter.key() == "data") {
            data = iter.value().toList();
            break;
        }
    }
    for (int i = 0; i < data.length(); i++) {
        OpenWeather* ow = new OpenWeather(cachePath, _log, this);
        ow->setup(data[i].toMap(), entities, notifications, api, configObj);

        QVariantMap d = data[i].toMap();
        d.insert("type", config.value("type").toString());
        returnData.insert(ow, d);
    }
    if (data.length() > 0) {
        emit createDone(returnData);
    }
}

WeatherItem OpenWeatherModel::toItem(const QString& units, const QString& iconUrl, bool current) {
    Q_UNUSED(units)
    WeatherItem item;
    QDate       dt = QDateTime::fromSecsSinceEpoch(date).date();
    QString     dayname = QDate::shortDayName(dt.dayOfWeek());

    if (dayname.endsWith('.')) {
        dayname = dayname.left(dayname.length() - 1);
    }

    item.setDate(dayname);
    item.setDescription(description);
    item.setImageurl(iconUrl + imageurl + "@2x.png");

    if (current) {
        item.setTemp(QString("%1 (%2 - %3) °C").arg(temp).arg(tempmin).arg(tempmax));
    } else {
        item.setTemp(QString("%1 - %2 °C").arg(tempmin).arg(tempmax));
    }

    if (rain > 0) {
        item.setRain(QString("%1 mm").arg(rain));
        item.setSnow("");
    } else {
        item.setRain("");
        if (snow > 0) {
            item.setSnow(QString("%1 mm").arg(snow));
        } else {
            item.setSnow("");
        }
    }
    if (wind > 0) {
        item.setWind(QString("%1 km/h").arg(wind));
    } else {
        item.setWind("");
    }
    item.setHumidity(QString("%1 %").arg(humidity));
    return item;
}
OpenWeatherModel::OpenWeatherModel()
    : date(0), temp(0), tempmin(0), tempmax(0), rain(0), snow(0), wind(0), humidity(0) {}

void OpenWeatherModel::fromCurrent(const QVariantMap& current) {
    date = current["dt"].toInt();
    QVariantMap main = current["main"].toMap();
    humidity = main["humidity"].toInt();
    temp = main["temp"].toInt();
    tempmin = main["temp_min"].toInt();
    tempmax = main["temp_max"].toInt();
    if (current.contains("rain")) {
        QVariantMap r = current["rain"].toMap();
        rain = r["3h"].toInt();
    }
    if (current.contains("snow")) {
        QVariantMap s = current["snow"].toMap();
        snow = s["3h"].toInt();
    }
    if (current.contains("wind")) {
        QVariantMap w = current["wind"].toMap();
        wind = w["speed"].toInt();
    }
    QVariantList weathers = current["weather"].toList();
    QVariantMap  weather = weathers.first().toMap();
    description = weather["description"].toString();
    imageurl = weather["icon"].toString();
}
void OpenWeatherModel::init(const OpenWeatherModel& model) {
    date = model.date;
    temp = model.temp;
    tempmin = model.tempmin;
    tempmax = model.tempmax;
    rain = model.rain;
    snow = model.snow;
    wind = model.wind;
    humidity = model.humidity;
    description = model.description;
    imageurl = model.imageurl;
}
void OpenWeatherModel::add(const OpenWeatherModel& model) {
    temp = model.temp;
    if (model.tempmin < tempmin) {
        tempmin = model.tempmin;
    }
    if (model.tempmax > tempmax) {
        tempmax = model.tempmax;
    }
    rain += model.rain;
    snow += model.snow;
    if (model.wind > wind) {
        wind = model.wind;
    }
    if (model.hour() / 3 == 15 / 3) {
        humidity = model.humidity;
        description = model.description;
        imageurl = model.imageurl;
    }
}

QList<OpenWeatherModel> OpenWeatherModel::fromForecast(const QVariantMap& forecast) {
    QList<OpenWeatherModel> list;
    QVariantList            l = forecast["list"].toList();
    for (QVariantList::iterator i = l.begin(); i != l.end(); ++i) {
        OpenWeatherModel model;
        model.fromCurrent(i->toMap());
        list.append(model);
    }
    return list;
}
void OpenWeatherModel::toDayForecast(QList<OpenWeatherModel>& perDay, const QList<OpenWeatherModel>& _3h) {
    OpenWeatherModel dayModel;
    int              day = 0;
    perDay.clear();
    for (QList<OpenWeatherModel>::const_iterator i = _3h.begin(); i != _3h.end(); ++i) {
        if (i->day() != day) {
            if (day != 0) {
                perDay.append(dayModel);
            }
            dayModel.init(*i);
            day = i->day();
        } else {
            dayModel.add(*i);
        }
    }
    if (day != 0) {
        perDay.append(dayModel);
    }
}

OpenWeather::OpenWeather(const QString& cacheDirectory, QLoggingCategory& log, QObject* parent)
    : _log(log),
      _cycleHours(0),
      _apiUrl("https://api.openweathermap.org/data/2.5/"),
      _iconUrl("https://openweathermap.org/img/wn/"),
      _notifications(nullptr),
      _imageCache(_iconUrl, cacheDirectory, _log, true),
      _nam(this) {
    setParent(parent);
    QObject::connect(&_imageCache, &ImageCache::allLoaded, this, &OpenWeather::onAllImagesLoaded);
}
OpenWeather::~OpenWeather() {}
void OpenWeather::setup(const QVariantMap& config, QObject* entities, QObject* notifications, QObject* api,
                        QObject* configObj) {
    Q_UNUSED(api)
    Q_UNUSED(configObj)
    Integration::setup(config, entities);

    for (QVariantMap::const_iterator iter = config.begin(); iter != config.end(); ++iter) {
        if (iter.key() == "key") {
            _key = iter.value().toString();
        } else if (iter.key() == "language") {
            _language = iter.value().toString();
        } else if (iter.key() == "units") {
            _units = iter.value().toString();
        } else if (iter.key() == "cyclehours") {
            _cycleHours = iter.value().toInt();
        }
    }
    _requestTimer.setInterval(1000 * 3600);
    _requestTimer.setSingleShot(false);
    _requestTimer.start();
    QObject::connect(&_requestTimer, &QTimer::timeout, this, [=]() {
        if (m_state != DISCONNECTED) {
            getAll();
        }
    });

    _notifications = qobject_cast<NotificationsInterface*>(notifications);
    qCDebug(_log) << "setup";
}

void OpenWeather::getCurrent(WeatherContext* context) {
    Q_ASSERT(context != nullptr);
    QString path =
        QString("weather?q=%1&APPID=%2&units=%3&lang=%4").arg(context->entity->entity_id(), _key, _units, _language);
    QNetworkRequest request(_apiUrl + path);
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    QNetworkReply* reply = _nam.get(request);
    QObject::connect(reply, &QNetworkReply::finished, this, [=]() {
        QString answer = reply->readAll();
        // convert to json
        QJsonParseError parseerror;
        QJsonDocument   doc = QJsonDocument::fromJson(answer.toUtf8(), &parseerror);
        if (parseerror.error != QJsonParseError::NoError) {
            jsonError("JSON error : " + parseerror.errorString());
            return;
        }
        QVariantMap map = doc.toVariant().toMap();
        onReplyCurrent(context, map);
    });
}
void OpenWeather::getForecast(WeatherContext* context) {
    Q_ASSERT(context != nullptr);
    QString path =
        QString("forecast?q=%1&APPID=%2&units=%3&lang=%4").arg(context->entity->entity_id(), _key, _units, _language);
    QNetworkRequest request(_apiUrl + path);
    request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/json");
    QNetworkReply* reply = _nam.get(request);
    QObject::connect(reply, &QNetworkReply::finished, this, [=]() {
        QString answer = reply->readAll();
        // convert to json
        QJsonParseError parseerror;
        QJsonDocument   doc = QJsonDocument::fromJson(answer.toUtf8(), &parseerror);
        if (parseerror.error != QJsonParseError::NoError) {
            jsonError("JSON error : " + parseerror.errorString());
            return;
        }
        QVariantMap map = doc.toVariant().toMap();
        onReplyForecast(context, map);
    });
}
void OpenWeather::getAll() {
    QDateTime now = QDateTime::currentDateTimeUtc();
    if (now >= _nextRequest) {
        _nextRequest = now.addSecs(_cycleHours * 3600);
        for (int i = 0; i < _contexts.length(); i++) {
            WeatherContext& context = _contexts[i];
            getCurrent(&context);
        }
    }
}

void OpenWeather::connect() {
    if (_contexts.count() == 0) {
        QList<EntityInterface*> entities = m_entities->getByIntegration(integrationId());
        int                     idx = 0;
        for (QList<EntityInterface*>::Iterator i = entities.begin(); i != entities.end(); ++i, idx++) {
            _contexts.append(WeatherContext(idx, *i));
        }
    }
    qCDebug(_log) << "connect";
    setState(CONNECTING);

    getAll();
}
void OpenWeather::disconnect() {
    qCDebug(_log) << "disconnect";
    setState(DISCONNECTED);
}
void OpenWeather::leaveStandby() { getAll(); }
void OpenWeather::sendCommand(const QString& type, const QString& id, int cmd, const QVariant& param) {
    if (_log.isDebugEnabled()) {
        qCDebug(_log) << "sendCommand " << type << " " << id << " " << cmd << " " << param.toString();
    }
}
void OpenWeather::jsonError(const QString& error) {
    Q_UNUSED(error)
    qCWarning(_log) << "Error:" << error;
}
void OpenWeather::onReplyCurrent(WeatherContext* context, QVariantMap& result) {
    if (state() != CONNECTED) {
        setState(CONNECTED);
    }
    Q_ASSERT(context != nullptr);
    context->current.fromCurrent(result);
    context->entity->setState(WeatherDef::ONLINE);
    WeatherInterface* wi = static_cast<WeatherInterface*>(context->entity->getSpecificInterface());
    WeatherItem       item = context->current.toItem(_units, _iconUrl, true);
    wi->setCurrent(item);
    getForecast(context);
}
void OpenWeather::onReplyForecast(WeatherContext* context, QVariantMap& result) {
    bool ready = true;
    if (state() != CONNECTED) {
        setState(CONNECTED);
    }
    Q_ASSERT(context != nullptr);
    context->entity->setState(WeatherDef::ONLINE);
    QList<OpenWeatherModel> forecast = OpenWeatherModel::fromForecast(result);
    OpenWeatherModel::toDayForecast(context->forecast, forecast);

    context->forecastWaitForImages.clear();

    WeatherItem todayitem = context->current.toItem(_units, _iconUrl, true);
    if (!applyImageCache(todayitem, context->current)) {
        ready = false;
    }
    context->forecastWaitForImages.append(todayitem);
    for (QList<OpenWeatherModel>::iterator i = context->forecast.begin(); i != context->forecast.end(); ++i) {
        if (context->current.day() != i->day()) {
            WeatherItem dayitem = i->toItem(_units, _iconUrl, false);
            if (!applyImageCache(dayitem, *i)) {
                ready = false;
            }
            context->forecastWaitForImages.append(dayitem);
        }
        if (context->forecastWaitForImages.count() == 5) {
            break;
        }
    }
    if (ready) {
        if (_log.isDebugEnabled()) {
            qCDebug(_log) << "images ready, update " << context->entity->entity_id();
        }
        WeatherInterface* wi = static_cast<WeatherInterface*>(context->entity->getSpecificInterface());
        _model.addItems(context->forecastWaitForImages);
        wi->setForecast(&_model);
        context->forecastWaitForImages.clear();
    }
}
void OpenWeather::onAllImagesLoaded() {
    for (int i = 0; i < _contexts.length(); i++) {
        WeatherContext& context = _contexts[i];
        if (context.forecastWaitForImages.count() > 0) {
            if (_log.isDebugEnabled()) {
                qCDebug(_log) << "images loaded, update " << context.entity->entity_id();
            }
            WeatherInterface* wi = static_cast<WeatherInterface*>(context.entity->getSpecificInterface());
            _model.addItems(context.forecastWaitForImages);
            wi->setForecast(&_model);
            context.forecastWaitForImages.clear();
        }
    }
}
bool OpenWeather::applyImageCache(WeatherItem& item, OpenWeatherModel& weatherModel) {
    QString filePath;
    bool    ready = _imageCache.get(weatherModel.imageurl + "@2x.png", filePath);
    item.setImageurl(filePath);
    return ready;
}