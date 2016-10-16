#include "QtLocation/private/qgeocameracapabilities_p.h"
#include "qgeotiledmappingmanagerenginegooglemaps.h"
#include "qgeotiledmapgooglemaps.h"
#include "qgeotilefetchergooglemaps.h"
#include "QtLocation/private/qgeotilespec_p.h"
#include "QtLocation/private/qgeofiletilecache_p.h"

#include <QDebug>
#include <QDir>
#include <QVariant>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/qmath.h>
#include <QtCore/qstandardpaths.h>

QT_BEGIN_NAMESPACE

QGeoTiledMappingManagerEngineGooglemaps::QGeoTiledMappingManagerEngineGooglemaps(const QVariantMap &parameters,
    QGeoServiceProvider::Error *error,
    QString *errorString)
    : QGeoTiledMappingManagerEngine()
{
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    QGeoCameraCapabilities capabilities;

    capabilities.setMinimumZoomLevel(0.0);
    capabilities.setMaximumZoomLevel(21.0);

    setCameraCapabilities(capabilities);

    int tile = parameters.value(QStringLiteral("googlemaps.maps.tilesize"), 256).toInt();

    setTileSize(QSize(tile, tile));

    QList<QGeoMapType> types;
    types << QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Normal map view in daylight mode"), false, false, 1);
    types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite Map"), tr("Satellite map view in daylight mode"), false, false, 2);
    types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain Map"), tr("Terrain map view in daylight mode"), false, false, 3);
    types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid Map"), tr("Satellite map view with streets in daylight mode"), false, false, 4);
    setSupportedMapTypes(types);

    QGeoTileFetcherGooglemaps *fetcher = new QGeoTileFetcherGooglemaps(parameters, this, tileSize());
    setTileFetcher(fetcher);

    if (parameters.contains(QStringLiteral("googlemaps.cachefolder")))
        m_cacheDirectory = parameters.value(QStringLiteral("googlemaps.cachefolder")).toString().toLatin1();
    else
        m_cacheDirectory = QAbstractGeoTileCache::baseCacheDirectory() + QLatin1String("googlemaps");

    QAbstractGeoTileCache *tileCache = new QGeoFileTileCache(m_cacheDirectory);
    tileCache->setMaxDiskUsage(100 * 1024 * 1024);
    setTileCache(tileCache);

    populateMapSchemes();
    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoTiledMappingManagerEngineGooglemaps::~QGeoTiledMappingManagerEngineGooglemaps()
{
}

void QGeoTiledMappingManagerEngineGooglemaps::populateMapSchemes()
{
    m_mapSchemes[0] = QStringLiteral("roadmap");
    m_mapSchemes[1] = QStringLiteral("roadmap");
    m_mapSchemes[2] = QStringLiteral("satellite");
    m_mapSchemes[3] = QStringLiteral("terrain");
    m_mapSchemes[4] = QStringLiteral("hybrid");
}

QString QGeoTiledMappingManagerEngineGooglemaps::getScheme(int mapId)
{
    return m_mapSchemes[mapId];
}

QGeoMap *QGeoTiledMappingManagerEngineGooglemaps::createMap()
{
    return new QGeoTiledMapGooglemaps(this);
}

QT_END_NAMESPACE

