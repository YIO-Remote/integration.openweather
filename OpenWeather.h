#define USE_MODEL 1
#pragma once
#include <QObject>
#include <QLoggingCategory>
#include "../remote-software/sources/integrations/integration.h"
#include "../remote-software/sources/integrations/integrationinterface.h"
#include "../remote-software/sources/entities/entitiesinterface.h"
#include "../remote-software/sources/notificationsinterface.h"
#include "RestClient.h"
#include "ImageCache.h"

class OpenWeatherFactory : public IntegrationInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "YIO.IntegrationInterface" FILE "openweather.json")
    Q_INTERFACES(IntegrationInterface)

public:
    explicit OpenWeatherFactory(QObject* parent = nullptr);
    virtual ~OpenWeatherFactory() override {
    }

    void        create              (const QVariantMap& config, QObject *entities, QObject *notifications, QObject* api, QObject *configObj) override;

private:
    QLoggingCategory    _log;
};

struct WeatherModel {
    WeatherModel();
    int         date;
    QString     description;
    QString     imageurl;
    int         temp;
    int         tempmin;
    int         tempmax;
    int         rain;
    int         snow;
    int         wind;
    int         humidity;

    int         day() const     { return date / 86400; }
    int         hour() const    { return (date % 86400) / 3600; }
    QVariantMap toCurrentMap    (const QString& units, const QString& iconUrl);
    QVariantMap toDayMap        (const QString& units, const QString& iconUrl);
    void        fromCurrent     (const QVariantMap& current);
    void        init            (const WeatherModel& model);
    void        add             (const WeatherModel& model);

    static QList<WeatherModel>  fromForecast(const QVariantMap& forecast);
    static void toDayForecast(QList<WeatherModel>& perDay, const QList<WeatherModel>& _3h);
};

class OpenWeather : public Integration
{
    Q_OBJECT
public:
    explicit	OpenWeather         (const QString& cacheDirectory, QObject* parent = nullptr);
    virtual     ~OpenWeather        () override;

    Q_INVOKABLE void setup  	    (const QVariantMap& config, QObject *entities, QObject *notifications, QObject* api, QObject *configObj);
    Q_INVOKABLE void connect	    ();
    Q_INVOKABLE void disconnect	    ();

public slots:
    void        sendCommand         (const QString& type, const QString& entity_id, const QString& command, const QVariant& param);
    void        onError             (const QString& error);
    void        onReplyCurrent      (QVariantMap result, QVariant arg);
    void        onReplyForecast     (QVariantMap result, QVariant arg);
    void        onAllImagesLoaded   ();

private:
    static QLoggingCategory         _log;

    struct WeatherContext  {
        WeatherContext () :
            index(0),
            entity(nullptr)
        {}
        WeatherContext (int index, EntityInterface* entity) :
            index(index),
            entity(entity)
        {}
        int                 index;
        EntityInterface*    entity;
        WeatherModel        current;
        QList<WeatherModel> forecast;
        QVariantList        forecastWaitForImages;
    };
    bool applyImageCache            (QVariantMap& map, WeatherModel& weatherModel);
    void getCurrent                 (WeatherContext &context);
    void getForecast                (WeatherContext &context);

    QString                         _apiUrl;
    QString                         _iconUrl;
    QString                         _key;
    QString                         _language;
    QString                         _units;
    QList<WeatherContext>           _contexts;
    NotificationsInterface*         _notifications;
    EntitiesInterface*              _entities;
    RestClient                      _restClient;
    ImageCache                      _imageCache;
};
