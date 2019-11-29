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
