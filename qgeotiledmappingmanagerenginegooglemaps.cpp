#include "QtLocation/private/qgeocameracapabilities_p.h"
#include "qgeotiledmappingmanagerenginegooglemaps.h"
#include "qgeotiledmapgooglemaps.h"
#include "qgeotilefetchergooglemaps.h"
#include "QtLocation/private/qgeotilespec_p.h"
#if QT_VERSION < QT_VERSION_CHECK(5,6,0)
#include <QStandardPaths>
#include "QtLocation/private/qgeotilecache_p.h"
#else
#include "QtLocation/private/qgeofiletilecache_p.h"
#endif

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
#if QT_VERSION < QT_VERSION_CHECK(5,9,0)
    types << QGeoMapType(QGeoMapType::StreetMap, tr("Road Map"), tr("Normal map view in daylight mode"), false, false, 1);
    types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite"), tr("Satellite map view in daylight mode"), false, false, 2);
    types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain"), tr("Terrain map view in daylight mode"), false, false, 3);
    types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid"), tr("Satellite map view with streets in daylight mode"), false, false, 4);
#elif QT_VERSION < QT_VERSION_CHECK(5,10,0)
    types << QGeoMapType(QGeoMapType::StreetMap, tr("Road Map"), tr("Normal map view in daylight mode"), false, false, 1, "googlemaps");
    types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite"), tr("Satellite map view in daylight mode"), false, false, 2, "googlemaps");
    types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain"), tr("Terrain map view in daylight mode"), false, false, 3, "googlemaps");
    types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid"), tr("Satellite map view with streets in daylight mode"), false, false, 4, "googlemaps");
#else
    types << QGeoMapType(QGeoMapType::StreetMap, tr("Road Map"), tr("Normal map view in daylight mode"), false, false, 1, "googlemaps", capabilities, parameters);
    types << QGeoMapType(QGeoMapType::SatelliteMapDay, tr("Satellite"), tr("Satellite map view in daylight mode"), false, false, 2, "googlemaps", capabilities, parameters);
    types << QGeoMapType(QGeoMapType::TerrainMap, tr("Terrain"), tr("Terrain map view in daylight mode"), false, false, 3, "googlemaps", capabilities, parameters);
    types << QGeoMapType(QGeoMapType::HybridMap, tr("Hybrid"), tr("Satellite map view with streets in daylight mode"), false, false, 4, "googlemaps", capabilities, parameters);
#endif
    setSupportedMapTypes(types);

    QGeoTileFetcherGooglemaps *fetcher = new QGeoTileFetcherGooglemaps(parameters, this, tileSize());
    setTileFetcher(fetcher);

    if (parameters.contains(QStringLiteral("googlemaps.cachefolder")))
        m_cacheDirectory = parameters.value(QStringLiteral("googlemaps.cachefolder")).toString();

    const int szCache = 100 * 1024 * 1024;
#if QT_VERSION < QT_VERSION_CHECK(5,6,0)
    if (m_cacheDirectory.isEmpty())
        m_cacheDirectory = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("googlemaps");
    QGeoTileCache *tileCache = createTileCacheWithDir(m_cacheDirectory);
    if (tileCache)
        tileCache->setMaxDiskUsage(szCache);
#else
    if (m_cacheDirectory.isEmpty())
        m_cacheDirectory = QAbstractGeoTileCache::baseCacheDirectory() + QLatin1String("googlemaps");
    QAbstractGeoTileCache *tileCache = new QGeoFileTileCache(m_cacheDirectory);
    tileCache->setMaxDiskUsage(szCache);
    setTileCache(tileCache);
#endif

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoTiledMappingManagerEngineGooglemaps::~QGeoTiledMappingManagerEngineGooglemaps()
{
}

QGeoMap *QGeoTiledMappingManagerEngineGooglemaps::createMap()
{
    return new QGeoTiledMapGooglemaps(this);
}

QT_END_NAMESPACE

