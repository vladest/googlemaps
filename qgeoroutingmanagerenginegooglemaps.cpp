#include "qgeoroutingmanagerenginegooglemaps.h"
#include "qgeoroutereplygooglemaps.h"

#include <QtCore/QUrlQuery>

#include <QtCore/QDebug>

QGeoRoutingManagerEngineGooglemaps::QGeoRoutingManagerEngineGooglemaps(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString)
:   QGeoRoutingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("googlemaps.useragent")))
        m_userAgent = parameters.value(QStringLiteral("googlemaps.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    m_urlPrefix = QStringLiteral("https://maps.googleapis.com/maps/api/directions/json");
    m_apiKey = parameters.value(QStringLiteral("googlemaps.route.apikey")).toString();

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoRoutingManagerEngineGooglemaps::~QGeoRoutingManagerEngineGooglemaps()
{
}

QGeoRouteReply* QGeoRoutingManagerEngineGooglemaps::calculateRoute(const QGeoRouteRequest &request)
{
    QNetworkRequest networkRequest;
    networkRequest.setRawHeader("User-Agent", m_userAgent);

    if (m_apiKey.isEmpty()) {
        QGeoRouteReply *reply = new QGeoRouteReply(QGeoRouteReply::UnsupportedOptionError, "Set googlemaps.route.apikey with google maps application key, supporting directions", this);
        emit error(reply, reply->error(), reply->errorString());
        return reply;
    }

    QUrl url(m_urlPrefix);
    QUrlQuery query;
    QStringList waypoints;

    foreach (const QGeoCoordinate &c, request.waypoints()) {
        QString scoord = QString::number(c.latitude()) + QLatin1Char(',') + QString::number(c.longitude());
        if (c == request.waypoints().first())
            query.addQueryItem(QStringLiteral("origin"), scoord);
        else if (c == request.waypoints().last())
            query.addQueryItem(QStringLiteral("destination"), scoord);
        else
            waypoints.append(scoord);
    }
    if (waypoints.size() > 0)
        query.addQueryItem(QStringLiteral("waypoints"), waypoints.join("|"));


    if (request.travelModes() & QGeoRouteRequest::CarTravel)
        query.addQueryItem(QStringLiteral("mode"), QStringLiteral("driving"));
    if (request.travelModes() & QGeoRouteRequest::PedestrianTravel)
        query.addQueryItem(QStringLiteral("mode"), QStringLiteral("walking"));
    if (request.travelModes() & QGeoRouteRequest::BicycleTravel)
        query.addQueryItem(QStringLiteral("mode"), QStringLiteral("bicycling"));
    if (request.travelModes() & QGeoRouteRequest::PublicTransitTravel)
        query.addQueryItem(QStringLiteral("mode"), QStringLiteral("transit"));

    if (request.numberAlternativeRoutes() > 1)
        query.addQueryItem(QStringLiteral("alternatives"), QStringLiteral("true"));

    QStringList avoidList;
    foreach (QGeoRouteRequest::FeatureType routeFeature, request.featureTypes()) {
        QGeoRouteRequest::FeatureWeight weigth = request.featureWeight(routeFeature);
        if (weigth == QGeoRouteRequest::AvoidFeatureWeight
                || weigth == QGeoRouteRequest::DisallowFeatureWeight) {
            if (routeFeature == QGeoRouteRequest::TollFeature)
                avoidList.append(QStringLiteral("tolls"));
            if (routeFeature == QGeoRouteRequest::HighwayFeature)
                avoidList.append(QStringLiteral("highways"));
            if (routeFeature == QGeoRouteRequest::FerryFeature)
                avoidList.append(QStringLiteral("ferries"));
        }
    }
    if (avoidList.size() > 0)
        query.addQueryItem(QStringLiteral("avoid"), avoidList.join("|"));


    if (QLocale::MetricSystem == measurementSystem())
        query.addQueryItem(QStringLiteral("units"), QStringLiteral("metric"));
    else
        query.addQueryItem(QStringLiteral("units"), QStringLiteral("imperial"));

    const QLocale loc(locale());

    if (QLocale::C != loc.language() && QLocale::AnyLanguage != loc.language()) {
        query.addQueryItem(QStringLiteral("language"), loc.name());
    }

    query.addQueryItem(QStringLiteral("key"), m_apiKey);


    url.setQuery(query);
    qDebug() << url;
    networkRequest.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(networkRequest);

    QGeoRouteReplyGooglemaps *routeReply = new QGeoRouteReplyGooglemaps(reply, request, this);

    connect(routeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(routeReply, SIGNAL(error(QGeoRouteReply::Error,QString)),
            this, SLOT(replyError(QGeoRouteReply::Error,QString)));

    return routeReply;
}

void QGeoRoutingManagerEngineGooglemaps::replyFinished()
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoRoutingManagerEngineGooglemaps::replyError(QGeoRouteReply::Error errorCode,
                                             const QString &errorString)
{
    QGeoRouteReply *reply = qobject_cast<QGeoRouteReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}
