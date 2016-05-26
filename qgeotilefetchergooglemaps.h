#ifndef QGEOTILEFETCHERGOOGLEMAPS_H
#define QGEOTILEFETCHERGOOGLEMAPS_H

#include "qgeoserviceproviderplugingooglemaps.h"

#include <QtLocation/private/qgeotilefetcher_p.h>
#include <QMutex>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE

class QGeoTiledMapReply;
class QGeoTileSpec;
class QGeoTiledMappingManagerEngine;
class QGeoTiledMappingManagerEngineGooglemaps;
class QNetworkAccessManager;

class QGeoTileFetcherGooglemaps : public QGeoTileFetcher
{
    Q_OBJECT

public:
    QGeoTileFetcherGooglemaps(const QVariantMap &parameters,
                         QGeoTiledMappingManagerEngineGooglemaps *engine, const QSize &tileSize);
    ~QGeoTileFetcherGooglemaps();

    QGeoTiledMapReply *getTileImage(const QGeoTileSpec &spec);

private:
    QString _getURL(int type, int x, int y, int zoom);
    void _tryCorrectGoogleVersions(QNetworkAccessManager *networkManager);
    void _getSecGoogleWords(int x, int y, QString &sec1, QString &sec2);

private slots:
    void _networkReplyError(QNetworkReply::NetworkError error);
    void _replyDestroyed();
    void _googleVersionCompleted();

private:
    Q_DISABLE_COPY(QGeoTileFetcherGooglemaps)

    QNetworkAccessManager *m_networkManager;

    QPointer<QGeoTiledMappingManagerEngineGooglemaps> m_engineGooglemaps;
    QSize m_tileSize;
    QString m_apiKey;
    QString m_signature;
    QString m_client;
    QString m_baseUri;

    int             _timeout;
    bool            _googleVersionRetrieved;
    QNetworkReply*  _googleReply;
    QMutex          _googleVersionMutex;
    QByteArray      _userAgent;
    QString         _language;

    // Google version strings
    QString         _versionGoogleMap;
    QString         _versionGoogleSatellite;
    QString         _versionGoogleLabels;
    QString         _versionGoogleTerrain;
    QString         _secGoogleWord;

    QNetworkRequest netRequest;
};

QT_END_NAMESPACE

#endif
