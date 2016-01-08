#include "qgeoserviceproviderplugingooglemaps.h"
#include "qgeocodingmanagerenginegooglemaps.h"
#include "qgeoroutingmanagerenginegooglemaps.h"
#include "qplacemanagerenginegooglemaps.h"
#include "qgeotiledmappingmanagerenginegooglemaps.h"


QGeoCodingManagerEngine *QGeoServiceProviderFactoryGooglemaps::createGeocodingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoCodingManagerEngineGooglemaps(parameters, error, errorString);
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryGooglemaps::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoRoutingManagerEngineGooglemaps(parameters, error, errorString);
}

QPlaceManagerEngine *QGeoServiceProviderFactoryGooglemaps::createPlaceManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QPlaceManagerEngineGooglemaps(parameters, error, errorString);
}

QGeoMappingManagerEngine *QGeoServiceProviderFactoryGooglemaps::createMappingManagerEngine(
        const QVariantMap &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    return new QGeoTiledMappingManagerEngineGooglemaps(parameters, error, errorString);
}
