#ifndef QGEOROUTINGMANAGERENGINEGOOGLEMAPS_H
#define QGEOROUTINGMANAGERENGINEGOOGLEMAPS_H

#include <QtLocation/QGeoServiceProvider>
#include <QtLocation/QGeoRoutingManagerEngine>

class QNetworkAccessManager;

class QGeoRoutingManagerEngineGooglemaps : public QGeoRoutingManagerEngine
{
    Q_OBJECT

public:
    QGeoRoutingManagerEngineGooglemaps(const QVariantMap &parameters,
                                QGeoServiceProvider::Error *error,
                                QString *errorString);
    ~QGeoRoutingManagerEngineGooglemaps();

    QGeoRouteReply *calculateRoute(const QGeoRouteRequest &request);

private Q_SLOTS:
    void replyFinished();
    void replyError(QGeoRouteReply::Error errorCode, const QString &errorString);

private:
    QNetworkAccessManager *m_networkManager;
    QByteArray m_userAgent;
    QString m_urlPrefix;
    QString m_apiKey;
};

#endif // QGEOROUTINGMANAGERENGINEGOOGLEMAPS_H

