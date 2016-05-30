#include "qgeocodingmanagerenginegooglemaps.h"
#include "qgeocodereplygooglemaps.h"

#include <QtCore/QVariantMap>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtCore/QLocale>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoAddress>
#include <QtPositioning/QGeoShape>
#include <QtPositioning/QGeoRectangle>

static QString addressToQuery(const QGeoAddress &address)
{
    return address.street() + QStringLiteral(",+") +
            address.district() + QStringLiteral(",+") +
            address.city() + QStringLiteral(",+") +
            address.state() + QStringLiteral(",+") +
            address.country();
}

static QString coordinateToQuery(const QGeoCoordinate &coordinate)
{
    return QString::number(coordinate.latitude()) + QStringLiteral(",") +
           QString::number(coordinate.longitude());
}

QGeoCodingManagerEngineGooglemaps::QGeoCodingManagerEngineGooglemaps(const QVariantMap &parameters,
                                                                     QGeoServiceProvider::Error *error,
                                                                     QString *errorString)
    :   QGeoCodingManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this))
{
    if (parameters.contains(QStringLiteral("googlemaps.useragent")))
        m_userAgent = parameters.value(QStringLiteral("googlemaps.useragent")).toString().toLatin1();
    else
        m_userAgent = "Qt Location based application";

    m_apiKey = parameters.value(QStringLiteral("googlemaps.geocode.apikey")).toString();

    m_urlPrefix = QStringLiteral("https://maps.googleapis.com/maps/api/geocode/json");

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoCodingManagerEngineGooglemaps::~QGeoCodingManagerEngineGooglemaps()
{
}

QGeoCodeReply *QGeoCodingManagerEngineGooglemaps::geocode(const QGeoAddress &address, const QGeoShape &bounds)
{
    return geocode(addressToQuery(address), -1, -1, bounds);
}

QGeoCodeReply *QGeoCodingManagerEngineGooglemaps::geocode(const QString &address, int limit, int offset, const QGeoShape &bounds)
{
    Q_UNUSED(offset)
    Q_UNUSED(limit)

    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QUrl url(m_urlPrefix);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("address"), address);
    query.addQueryItem(QStringLiteral("key"), m_apiKey);
    if (bounds.isValid() && !bounds.isEmpty() && bounds.type() != QGeoShape::UnknownType) {
        if (bounds.type() == QGeoShape::RectangleType) {
            const QGeoRectangle &r = static_cast<const QGeoRectangle&>(bounds);
            query.addQueryItem(QStringLiteral("bounds"),
                               (coordinateToQuery(r.topRight()) + "|" + coordinateToQuery(r.bottomLeft())));
        }
    }
    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);

    QGeoCodeReplyGooglemaps *geocodeReply = new QGeoCodeReplyGooglemaps(reply, this);

    connect(geocodeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(geocodeReply, SIGNAL(error(QGeoCodeReply::Error,QString)),
            this, SLOT(replyError(QGeoCodeReply::Error,QString)));

    return geocodeReply;
}

QGeoCodeReply *QGeoCodingManagerEngineGooglemaps::reverseGeocode(const QGeoCoordinate &coordinate,
                                                                 const QGeoShape &bounds)
{
    Q_UNUSED(bounds)

    QNetworkRequest request;
    request.setRawHeader("User-Agent", m_userAgent);

    QUrl url(m_urlPrefix);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("latlng"), coordinateToQuery(coordinate));
    query.addQueryItem(QStringLiteral("key"), m_apiKey);

    url.setQuery(query);
    request.setUrl(url);

    QNetworkReply *reply = m_networkManager->get(request);

    QGeoCodeReplyGooglemaps *geocodeReply = new QGeoCodeReplyGooglemaps(reply, this);

    connect(geocodeReply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(geocodeReply, SIGNAL(error(QGeoCodeReply::Error,QString)),
            this, SLOT(replyError(QGeoCodeReply::Error,QString)));

    return geocodeReply;
}

void QGeoCodingManagerEngineGooglemaps::replyFinished()
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QGeoCodingManagerEngineGooglemaps::replyError(QGeoCodeReply::Error errorCode, const QString &errorString)
{
    QGeoCodeReply *reply = qobject_cast<QGeoCodeReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}
