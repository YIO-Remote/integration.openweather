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
