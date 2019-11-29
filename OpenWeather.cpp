#include <QDir>
#include "OpenWeather.h"
#include "../remote-software/sources/configinterface.h"
#include "../remote-software/sources/entities/weatherinterface.h"

OpenWeatherFactory::OpenWeatherFactory(QObject* parent) :
    _log("openweather")
{
    Q_UNUSED(parent)
}

void OpenWeatherFactory::create(const QVariantMap &config, QObject *entities, QObject *notifications, QObject *api, QObject *configObj)
{
    QMap<QObject *, QVariant>   returnData;
    QVariantList                data;

    ConfigInterface* configInterface = qobject_cast<ConfigInterface *>(configObj);
    QString cachePath = configInterface->getContextProperty ("configPath").toString() + "/openweather";
    if (!QDir(cachePath).exists())
        QDir().mkdir(cachePath);

    for (QVariantMap::const_iterator iter = config.begin(); iter != config.end(); ++iter) {
        if (iter.key() == "data") {
            data = iter.value().toList();
            break;
        }
    }
    for (int i = 0; i < data.length(); i++)
    {
        OpenWeather* ow = new OpenWeather(cachePath, this);
        ow->setup(data[i].toMap(), entities, notifications, api, configObj);

        QVariantMap d = data[i].toMap();
        d.insert("type", config.value("type").toString());
        returnData.insert(ow, d);
    }
    if (data.length() > 0)
        emit createDone(returnData);
}

QVariantMap WeatherModel::toCurrentMap(const QString& units, const QString& iconUrl) {
    Q_UNUSED(units)
    QVariantMap map;
    QDate dt = QDateTime::fromSecsSinceEpoch(date).date();
    QString dayname = QDate::shortDayName(dt.dayOfWeek());
    if (dayname.endsWith('.'))
        dayname = dayname.left(dayname.length() - 1);
    map["date"] = dayname;
    map["description"] = description;
    map["imageurl"] = iconUrl + imageurl + "@2x.png";
    map["temp"] = QString ("%1 (%2 - %3) °C").arg(temp).arg(tempmin).arg(tempmax);
    if (rain > 0) {
        map["rain"] = QString("%1 mm").arg(rain);
        map["snow"] = "";
    }
    else {
        map["rain"] = "";
        if (snow > 0)
            map["snow"] = QString("%1 mm").arg(snow);
        else
            map["snow"] = "";
    }
    if (wind > 0)
        map["wind"] = QString("%1 km/h").arg(wind);
    else
        map["wind"] = "";
    map["humidity"] = QString("%1 %").arg(humidity);
    return map;
}
QVariantMap WeatherModel::toDayMap(const QString& units, const QString& iconUrl) {
    Q_UNUSED(units)
    QVariantMap map;
    QDate dt = QDateTime::fromSecsSinceEpoch(date).date();
    QString dayname = QDate::shortDayName(dt.dayOfWeek());
    if (dayname.endsWith('.'))
        dayname = dayname.left(dayname.length() - 1);
    map["date"] = dayname;
    map["description"] = description;
    map["imageurl"] = iconUrl + imageurl + "@2x.png";
    map["temp"] = QString ("%1 - %2 °C").arg(tempmin).arg(tempmax);
    if (rain > 0) {
        map["rain"] = QString("%1 mm").arg(rain);
        map["snow"] = "";
    }
    else {
        map["rain"] = "";
        if (snow > 0)
            map["snow"] = QString("%1 mm").arg(snow);
        else
            map["snow"] = "";
    }
    if (wind > 0)
        map["wind"] = QString("%1 km/h").arg(wind);
    else
        map["wind"] = "";
    map["humidity"] = QString("%1 %").arg(humidity);
    return map;
}
WeatherModel::WeatherModel() :
    date(0),
    temp(0),
    tempmin(0),
    tempmax(0),
    rain(0),
    snow(0),
    wind(0),
    humidity(0)
{
}

void WeatherModel::fromCurrent(const QVariantMap &current) {
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
    QVariantMap weather = weathers.first().toMap();
    description = weather["description"].toString();
    imageurl = weather["icon"].toString();
}
void WeatherModel::init (const WeatherModel& model)
{
    date =          model.date;
    temp =          model.temp;
    tempmin =       model.tempmin;
    tempmax =       model.tempmax;
    rain =          model.rain;
    snow =          model.snow;
    wind =          model.wind;
    humidity =      model.humidity;
    description =   model.description;
    imageurl =      model.imageurl;
}
void WeatherModel::add (const WeatherModel& model)
{
    temp =          model.temp;
    if (model.tempmin < tempmin)
        tempmin = model.tempmin;
    if (model.tempmax > tempmax)
        tempmax = model.tempmax;
    rain +=         model.rain;
    snow +=         model.snow;
    if (model.wind > wind)
        wind =      model.wind;
    if (model.hour() / 3 == 15 / 3) {
        humidity =      model.humidity;
        description =   model.description;
        imageurl =      model.imageurl;
    }
}

QList<WeatherModel> WeatherModel::fromForecast(const QVariantMap& forecast)
{
    QList<WeatherModel> list;
    QVariantList l = forecast["list"].toList();
    for (QVariantList::iterator i = l.begin(); i != l.end(); ++i) {
        WeatherModel model;
        model.fromCurrent(i->toMap());
        list.append(model);
    }
    return list;
}
void WeatherModel::toDayForecast(QList<WeatherModel>& perDay, const QList<WeatherModel>& _3h)
{
    WeatherModel        dayModel;
    int day = 0;
    perDay.clear();
    for (QList<WeatherModel>::const_iterator i = _3h.begin(); i != _3h.end(); ++i) {
        if (i->day() != day) {
            if (day != 0)
                perDay.append(dayModel);
            dayModel.init(*i);
            day = i->day();
        }
        else {
            dayModel.add(*i);
        }
    }
    if (day != 0)
        perDay.append(dayModel);
}


QLoggingCategory OpenWeather::_log("roon");

OpenWeather::OpenWeather(const QString& cacheDirectory, QObject* parent) :
    _apiUrl("https://api.openweathermap.org/data/2.5/"),
    _iconUrl("https://openweathermap.org/img/wn/"),
    _notifications(nullptr),
    _entities(nullptr),
    _restClient(_apiUrl, this),
    _imageCache(_iconUrl, cacheDirectory, _log, true)
{
    setParent (parent);
    QObject::connect (&_imageCache, &ImageCache::allLoaded, this, &OpenWeather::onAllImagesLoaded);
}
OpenWeather::~OpenWeather()
{
}

void OpenWeather::setup (const QVariantMap& config, QObject *entities, QObject *notifications, QObject* api, QObject *configObj)
{
    Q_UNUSED(api)
    Q_UNUSED(configObj)

    _log.setEnabled(QtMsgType::QtDebugMsg, false);     // Default, only debug disabled
    for (QVariantMap::const_iterator iter = config.begin(); iter != config.end(); ++iter) {
        if (iter.key() == "friendly_name")
            setFriendlyName(iter.value().toString());
        else if (iter.key() == "id")
            setIntegrationId(iter.value().toString());
        else if (iter.key() == "key")
            _key = iter.value().toString();
        else if (iter.key() == "language")
            _language = iter.value().toString();
        else if (iter.key() == "units")
            _units = iter.value().toString();
        else if (iter.key() == "log") {
            const QString& severity = iter.value().toString();
            if (severity == "debug") {
                _log.setEnabled(QtMsgType::QtDebugMsg, true);
            }
            else if (severity == "info") {
                _log.setEnabled(QtMsgType::QtDebugMsg, false);
                _log.setEnabled(QtMsgType::QtInfoMsg, true);
            }
            else if (severity == "warning") {
                _log.setEnabled(QtMsgType::QtDebugMsg, false);
                _log.setEnabled(QtMsgType::QtInfoMsg, false);
                _log.setEnabled(QtMsgType::QtWarningMsg, true);
            }
        }
    }
    _notifications = qobject_cast<NotificationsInterface *> (notifications);
    _entities = qobject_cast<EntitiesInterface *> (entities);
}

void OpenWeather::getCurrent (WeatherContext &context) {
    QString path = QString("weather?q=%1&APPID=%2&units=%3&lang=%4").arg(context.entity->entity_id(), _key, _units, _language);
    _restClient.get (path, context.index, SLOT(onReplyCurrent));
}
void OpenWeather::getForecast (WeatherContext &context) {
    QString path = QString("forecast?q=%1&APPID=%2&units=%3&lang=%4").arg(context.entity->entity_id(), _key, _units, _language);
    _restClient.get (path, context.index, SLOT(onReplyForecast));
}


void OpenWeather::connect()
{
    QList<EntityInterface*> entities = _entities->getByIntegration(integrationId());
    int idx = 0;
    for (QList<EntityInterface*>::Iterator i = entities.begin(); i != entities.end(); ++i, idx++) {
        _contexts.append(WeatherContext(idx, *i));
    }
    qCDebug(_log) << "connect";
    setState(CONNECTING);

    // @@@
    getCurrent(_contexts[0]);
    //getForecast(_contexts[0]);
}
void OpenWeather::disconnect()
{
    qCDebug(_log) << "disconnect";
    setState(DISCONNECTED);
}
void OpenWeather::sendCommand(const QString& type, const QString& id, const QString& cmd, const QVariant& param)
{
    if (_log.isDebugEnabled())
        qCDebug(_log) << "sendCommand " << type << " " << id << " " << cmd << " " << param.toString();
}
void OpenWeather::onError (const QString& error)
{
    Q_UNUSED(error)
    qCWarning(_log) << "Error:" << error;
}
void OpenWeather::onReplyCurrent (QVariantMap result, QVariant arg)
{
    if (state() != CONNECTED)
        setState(CONNECTED);
    WeatherContext& context = _contexts[arg.toInt()];
    context.current.fromCurrent(result);
    QVariantMap map = context.current.toCurrentMap(_units, _iconUrl);
    context.entity->updateAttrByIndex(WeatherDef::CURRENT, map);

    getForecast(_contexts[0]);  // @@@
}
void OpenWeather::onReplyForecast(QVariantMap result, QVariant arg)
{
    bool ready = true;
    if (state() != CONNECTED)
        setState(CONNECTED);
    WeatherContext& context = _contexts[arg.toInt()];
    QList<WeatherModel> forecast = WeatherModel::fromForecast(result);
    WeatherModel::toDayForecast(context.forecast, forecast);

    context.forecastWaitForImages.clear();

    QVariantMap todaymap = context.current.toCurrentMap(_units, _iconUrl);
    if (!applyImageCache(todaymap, context.current))
        ready = false;
    context.forecastWaitForImages.append(todaymap);
    for (QList<WeatherModel>::iterator i = context.forecast.begin(); i != context.forecast.end(); ++i) {
        if (context.current.day() != i->day()) {
            QVariantMap daymap = i->toDayMap(_units, _iconUrl);
            if (!applyImageCache(daymap, *i))
                ready = false;
            context.forecastWaitForImages.append(daymap);
        }
        if (context.forecastWaitForImages.count() == 5)
            break;
    }
    if (ready) {
        if (_log.isDebugEnabled())
            qCDebug(_log) << "images ready, update " << context.entity->entity_id();
        context.entity->updateAttrByIndex(WeatherDef::FORECAST, context.forecastWaitForImages);
        context.forecastWaitForImages.clear();
    }
}
void OpenWeather::onAllImagesLoaded ()
{
    for (int i = 0; i < _contexts.length(); i++) {
        WeatherContext& context = _contexts[i];
        if (context.forecastWaitForImages.count() > 0) {
            if (_log.isDebugEnabled())
                qCDebug(_log) << "images loaded, update " << context.entity->entity_id();
            context.entity->updateAttrByIndex(WeatherDef::FORECAST, context.forecastWaitForImages);
            context.forecastWaitForImages.clear();
        }
    }
}
bool OpenWeather::applyImageCache(QVariantMap &map, WeatherModel &weatherModel) {
    QString filePath;
    bool ready = _imageCache.get(weatherModel.imageurl + "@2x.png", filePath);
    map["imageurl"] = filePath;
    return ready;
}
