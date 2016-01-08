#ifndef QGEOTILEFETCHERGOOGLEMAPS_H
#define QGEOTILEFETCHERGOOGLEMAPS_H

#include "qgeoserviceproviderplugingooglemaps.h"

#include <QtLocation/private/qgeotilefetcher_p.h>

QT_BEGIN_NAMESPACE

class QGeoTiledMapReply;
class QGeoTileSpec;
class QGeoTiledMappingManagerEngine;
class QGeoTiledMappingManagerEngineGooglemaps;
class QNetworkReply;
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
    Q_DISABLE_COPY(QGeoTileFetcherGooglemaps)

    QNetworkAccessManager *m_networkManager;

    QString getLanguageString() const;

    QPointer<QGeoTiledMappingManagerEngineGooglemaps> m_engineGooglemaps;
    QSize m_tileSize;
    QString m_apiKey;
    QString m_signature;
    QString m_client;
    QString m_baseUri;
};

QT_END_NAMESPACE

#endif
