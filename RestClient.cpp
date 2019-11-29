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
