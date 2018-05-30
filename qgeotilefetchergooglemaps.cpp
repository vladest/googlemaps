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
#include <QNetworkProxy>
#include <QtCore/QJsonDocument>
#include <QSslSocket>

#include <math.h>

#include <map>

QT_BEGIN_NAMESPACE

QGeoTileFetcherGooglemaps::QGeoTileFetcherGooglemaps(const QVariantMap &parameters,
                                           QGeoTiledMappingManagerEngineGooglemaps *engine,
                                           const QSize &tileSize)
:   QGeoTileFetcher(engine),
  m_networkManager(new QNetworkAccessManager(this)),
  m_engineGooglemaps(engine),
  m_tileSize(tileSize),
  _googleVersionRetrieved(false),
  _scale(1)
{
    if(parameters.contains(QStringLiteral("googlemaps.maps.apikey")))
        m_apiKey = parameters.value(QStringLiteral("googlemaps.maps.apikey")).toString();
    else
        m_apiKey = parameters.value(QStringLiteral("googlemaps.apikey")).toString();
    m_signature = parameters.value(QStringLiteral("googlemaps.maps.signature")).toString();
    m_client = parameters.value(QStringLiteral("googlemaps.maps.client")).toString();
    m_baseUri = QStringLiteral("http://maps.googleapis.com/maps/api/staticmap");
    if (parameters.contains(QStringLiteral("googlemaps.useragent")))
        _userAgent = parameters.value(QStringLiteral("googlemaps.useragent")).toString().toLatin1();
    else
        _userAgent = "";
    if (parameters.contains(QStringLiteral("googlemaps.maps.language"))) {
        _language = parameters.value(QStringLiteral("googlemaps.maps.language")).toString().toLatin1();
        if (_language.isEmpty())
           _language = "en-US";
    } else {
        QStringList langs = QLocale::system().uiLanguages();
        _language = (langs.length() > 0) ? langs[0] : "en-US";
    }

    if (parameters.contains(QStringLiteral("googlemaps.maps.highdpi")))
        _scale = (parameters.value(QStringLiteral("googlemaps.maps.highdpi")).toBool()) ? 2 : 1;

    // Google version strings
    _secGoogleWord               = "Galileo";
}

QGeoTileFetcherGooglemaps::~QGeoTileFetcherGooglemaps()
{
}

void QGeoTileFetcherGooglemaps::_getSessionToken()
{
    QUrl sessionUrl("https://www.googleapis.com/tile/v1/createSession");

    QUrlQuery queryItems;
    queryItems.addQueryItem("key", m_apiKey);
    queryItems.addQueryItem("mapType", "roadmap");
    queryItems.addQueryItem("language", _language);
    queryItems.addQueryItem("region", "de");

    sessionUrl.setQuery(queryItems);
    netRequest.setUrl(sessionUrl);
    QNetworkReply *sessionReply = m_networkManager->get(netRequest);


    if (sessionReply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(sessionReply->readAll());
    if (!document.isObject())
        return;

    QJsonObject object = document.object();
    QJsonValue status = object.value(QStringLiteral("session"));
    printf("%s", status.toString().toLatin1().data());
}

QGeoTiledMapReply *QGeoTileFetcherGooglemaps::getTileImage(const QGeoTileSpec &spec)
{
    QString surl = _getURL(spec.mapId(), spec.x(), spec.y(), spec.zoom());
    QUrl url(surl);

    netRequest.setUrl(url);

    QNetworkReply *netReply = m_networkManager->get(netRequest);

    QGeoTiledMapReply *mapReply = new QGeoMapReplyGooglemaps(netReply, spec);

    return mapReply;
}

void QGeoTileFetcherGooglemaps::_getSecGoogleWords(int x, int y, QString &sec1, QString &sec2)
{
    sec1 = ""; // after &x=...
    sec2 = ""; // after &zoom=...
    int seclen = ((x * 3) + y) % 8;
    sec2 = _secGoogleWord.left(seclen);
    if (y >= 10000 && y < 100000) {
        sec1 = "&s=";
    }
}

QString QGeoTileFetcherGooglemaps::_getURL(int type, int x, int y, int zoom)
{
    switch (type) {
    case 0:
    case 1: //Road Map
    {
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecGoogleWords(x, y, sec1, sec2);
        return QString("http://mt.google.com/vt/lyrs=m&hl=%1&x=%2%3&y=%4&z=%5&s=%6&scale=%7").arg(_language).arg(x).arg(sec1).arg(y).arg(zoom).arg(sec2).arg(_scale);
    }
    break;
    case 2: //Satallite Map
    {
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecGoogleWords(x, y, sec1, sec2);
        return QString("http://mt.google.com/vt/lyrs=s&hl=%1&x=%2%3&y=%4&z=%5&s=%6&scale=%7").arg(_language).arg(x).arg(sec1).arg(y).arg(zoom).arg(sec2).arg(_scale);
    }
    break;
    case 3: //Terrain Map
    {
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecGoogleWords(x, y, sec1, sec2);
        return QString("http://mt.google.com/vt/lyrs=p&hl=%1&x=%2%3&y=%4&z=%5&s=%6&scale=%7").arg(_language).arg(x).arg(sec1).arg(y).arg(zoom).arg(sec2).arg(_scale);
    }
    break;
    case 4: //Hybrid Map
    {
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecGoogleWords(x, y, sec1, sec2);
        return QString("http://mt.google.com/vt/lyrs=y&hl=%1&x=%2%3&y=%4&z=%5&s=%6&scale=%7").arg(_language).arg(x).arg(sec1).arg(y).arg(zoom).arg(sec2).arg(_scale);
    }
    break;
    }
    return "";
}

void QGeoTileFetcherGooglemaps::_networkReplyError(QNetworkReply::NetworkError error)
{
    qWarning() << "Could not connect to google maps. Error:" << error;
    if(_googleReply)
    {
        _googleReply->deleteLater();
        _googleReply = NULL;
    }
}

void QGeoTileFetcherGooglemaps::_replyDestroyed()
{
    _googleReply = NULL;
}

void QGeoTileFetcherGooglemaps::_googleVersionCompleted()
{
    if (!_googleReply || (_googleReply->error() != QNetworkReply::NoError)) {
        qDebug() << "Error collecting Google maps version info";
        return;
    }
    _googleReply->deleteLater();
    _googleReply = NULL;
}


void QGeoTileFetcherGooglemaps::_tryCorrectGoogleVersions(QNetworkAccessManager* networkManager)
{
    QMutexLocker locker(&_googleVersionMutex);
    if (_googleVersionRetrieved) {
        return;
    }
    _googleVersionRetrieved = true;
    if(networkManager)
    {
        QNetworkRequest qheader;
        QNetworkProxy proxy = networkManager->proxy();
        QNetworkProxy tProxy;
        tProxy.setType(QNetworkProxy::DefaultProxy);
        networkManager->setProxy(tProxy);
#ifndef QT_NO_SSL
        QSslConfiguration conf = qheader.sslConfiguration();
        conf.setPeerVerifyMode(QSslSocket::VerifyNone);
        qheader.setSslConfiguration(conf);
#endif
        QString url = "http://maps.google.com/maps/api/js?v=3.2&sensor=false";
        qheader.setUrl(QUrl(url));
        qheader.setRawHeader("User-Agent", _userAgent);
        _googleReply = networkManager->get(qheader);
        connect(_googleReply, &QNetworkReply::finished, this, &QGeoTileFetcherGooglemaps::_googleVersionCompleted);
        connect(_googleReply, &QNetworkReply::destroyed, this, &QGeoTileFetcherGooglemaps::_replyDestroyed);
        connect(_googleReply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                this, &QGeoTileFetcherGooglemaps::_networkReplyError);
        networkManager->setProxy(proxy);
    }
}

QT_END_NAMESPACE
