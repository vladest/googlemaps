#include "qgeotilefetchergooglemaps.h"
#include "qgeomapreplygooglemaps.h"
#include "qgeotiledmapgooglemaps.h"
#include "qgeotiledmappingmanagerenginegooglemaps.h"
#include <QtLocation/private/qgeotilespec_p.h>

#include <QDebug>
#include <QSize>
#include <QDir>
#include <QUrl>
#include <QUrlQuery>
#include <QTime>

#include <map>

QT_BEGIN_NAMESPACE

namespace
{

    double tilex2long(int x, int z)
    {
        return (double)x / pow(2.0, z) * 360.0 - 180.0;
    }

    double tiley2lat(int y, int z)
    {
        double n = M_PI - 2.0 * M_PI * (double)y / pow(2.0, z);
        return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
    }

    QGeoCoordinate xyzToCoord(int x, int y, int z)
    {
        return QGeoCoordinate(tiley2lat(y, z), tilex2long(x, z), 0.0);
    }

    QString sizeToStr(const QSize &size)
    {
        if (size.height() >= 512 || size.width() >= 512)
            return QStringLiteral("512x512");
        else if (size.height() >= 256 || size.width() >= 256)
            return QStringLiteral("256x256");
        else
            return QStringLiteral("128x128");   // 128 pixel tiles are deprecated.
    }

    QGeoCoordinate tileCenter(const QGeoTileSpec &spec) {
        int viewX0, viewY0, viewX1, viewY1;
        viewX0 = spec.x();
        viewY0 = spec.y();
        viewX1 = spec.x() + 1;
        viewY1 = spec.y() + 1;

        QGeoRectangle viewport(xyzToCoord(viewX0, viewY0, spec.zoom()), xyzToCoord(viewX1, viewY1, spec.zoom()));
        return viewport.center();
    }

    QString coordToStr(const QGeoCoordinate &coord)
    {
        //its important to have precision 8. otherwise tile will not be fitted correctly
        char format = 'g';
        return QString::number(coord.latitude(), format, 8) + "," + QString::number(coord.longitude(), format, 8);
    }

}

QGeoTileFetcherGooglemaps::QGeoTileFetcherGooglemaps(const QVariantMap &parameters,
                                           QGeoTiledMappingManagerEngineGooglemaps *engine,
                                           const QSize &tileSize)
:   QGeoTileFetcher(engine),
  m_networkManager(new QNetworkAccessManager(this)),
  m_engineGooglemaps(engine),
  m_tileSize(tileSize)
{
    m_apiKey = parameters.value(QStringLiteral("googlemaps.maps.apikey")).toString();
    m_signature = parameters.value(QStringLiteral("googlemaps.maps.signature")).toString();
    m_client = parameters.value(QStringLiteral("googlemaps.maps.client")).toString();
    m_baseUri = QStringLiteral("http://maps.googleapis.com/maps/api/staticmap");
}

QGeoTileFetcherGooglemaps::~QGeoTileFetcherGooglemaps()
{
}

QGeoTiledMapReply *QGeoTileFetcherGooglemaps::getTileImage(const QGeoTileSpec &spec)
{

    if (m_apiKey.isEmpty()) {
        QGeoTiledMapReply *reply = new QGeoTiledMapReply(QGeoTiledMapReply::UnknownError, "Set googlemaps.maps.apikey with google maps application key, supporting static maps", this);
        emit tileError(spec, reply->errorString());
        return reply;
    }
    QUrlQuery query;
    QUrl url(m_baseUri);
    query.addQueryItem("maptype", m_engineGooglemaps->getScheme(spec.mapId()));
    query.addQueryItem("size", sizeToStr(m_tileSize));
    query.addQueryItem("center", coordToStr(tileCenter(spec)));
    query.addQueryItem("zoom", QString::number(spec.zoom()));
    query.addQueryItem("format", "png");
    query.addQueryItem("key", m_apiKey);
    if (!m_signature.isEmpty())
        query.addQueryItem("signature", m_signature);
    if (!m_client.isEmpty())
        query.addQueryItem("client", m_client);

    url.setQuery(query);
    //qDebug() << "maps url:" << url;
    QNetworkRequest netRequest(url); // The extra pair of parens disambiguates this from a function declaration
    netRequest.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

    QNetworkReply *netReply = m_networkManager->get(netRequest);

    QGeoTiledMapReply *mapReply = new QGeoMapReplyGooglemaps(netReply, spec);

    return mapReply;
}

QString QGeoTileFetcherGooglemaps::getLanguageString() const
{
    if (!m_engineGooglemaps)
        return QStringLiteral("ENG");

    QLocale locale = m_engineGooglemaps.data()->locale();

    // English is the default, where no ln is specified. We hardcode the languages
    // here even though the entire list is updated automagically from the server.
    // The current languages are Arabic, Chinese, Simplified Chinese, English
    // French, German, Italian, Polish, Russian and Spanish. The default is English.
    // These are actually available from the same host under the URL: /maptiler/v2/info

    switch (locale.language()) {
    case QLocale::Arabic:
        return QStringLiteral("ARA");
    case QLocale::Chinese:
        if (locale.script() == QLocale::TraditionalChineseScript)
            return QStringLiteral("CHI");
        else
            return QStringLiteral("CHT");
    case QLocale::Dutch:
        return QStringLiteral("DUT");
    case QLocale::French:
        return QStringLiteral("FRE");
    case QLocale::German:
        return QStringLiteral("GER");
    case QLocale::Gaelic:
        return QStringLiteral("GLE");
    case QLocale::Greek:
        return QStringLiteral("GRE");
    case QLocale::Hebrew:
        return QStringLiteral("HEB");
    case QLocale::Hindi:
        return QStringLiteral("HIN");
    case QLocale::Indonesian:
        return QStringLiteral("IND");
    case QLocale::Italian:
        return QStringLiteral("ITA");
    case QLocale::Persian:
        return QStringLiteral("PER");
    case QLocale::Polish:
        return QStringLiteral("POL");
    case QLocale::Portuguese:
        return QStringLiteral("POR");
    case QLocale::Russian:
        return QStringLiteral("RUS");
    case QLocale::Sinhala:
        return QStringLiteral("SIN");
    case QLocale::Spanish:
        return QStringLiteral("SPA");
    case QLocale::Thai:
        return QStringLiteral("THA");
    case QLocale::Turkish:
        return QStringLiteral("TUR");
    case QLocale::Ukrainian:
        return QStringLiteral("UKR");
    case QLocale::Urdu:
        return QStringLiteral("URD");
    case QLocale::Vietnamese:
        return QStringLiteral("VIE");

    default:
        return QStringLiteral("ENG");
    }
    // No "lg" param means that we want English.
}

QT_END_NAMESPACE
