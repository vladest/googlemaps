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

#include <math.h>

#include <map>

QT_BEGIN_NAMESPACE

namespace
{

    int _getServerNum(int x, int y, int max)
    {
        return (x + 2 * y) % max;
    }

}

QGeoTileFetcherGooglemaps::QGeoTileFetcherGooglemaps(const QVariantMap &parameters,
                                           QGeoTiledMappingManagerEngineGooglemaps *engine,
                                           const QSize &tileSize)
:   QGeoTileFetcher(engine),
  m_networkManager(new QNetworkAccessManager(this)),
  m_engineGooglemaps(engine),
  m_tileSize(tileSize),
  _googleVersionRetrieved(false)
{
    m_apiKey = parameters.value(QStringLiteral("googlemaps.maps.apikey")).toString();
    m_signature = parameters.value(QStringLiteral("googlemaps.maps.signature")).toString();
    m_client = parameters.value(QStringLiteral("googlemaps.maps.client")).toString();
    m_baseUri = QStringLiteral("http://maps.googleapis.com/maps/api/staticmap");
    if (parameters.contains(QStringLiteral("googlemaps.useragent")))
        _userAgent = parameters.value(QStringLiteral("googlemaps.useragent")).toString().toLatin1();
    else
        _userAgent = "Mozilla/5.0 (X11; Linux i586; rv:31.0) Gecko/20100101 Firefox/31.0";

    QStringList langs = QLocale::system().uiLanguages();
    if (langs.length() > 0) {
        _language = langs[0];
    }

    // Google version strings
    _versionGoogleMap            = "m@338000000";
    _versionGoogleSatellite      = "198";
    _versionGoogleLabels         = "h@336";
    _versionGoogleTerrain        = "t@132,r@338000000";
    _secGoogleWord               = "Galileo";

    _tryCorrectGoogleVersions(m_networkManager);

    netRequest.setRawHeader("Referrer", "https://www.google.com/maps/preview");
    netRequest.setRawHeader("Accept", "*/*");
    netRequest.setRawHeader("User-Agent", _userAgent);

}

QGeoTileFetcherGooglemaps::~QGeoTileFetcherGooglemaps()
{
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
    case 1:
    {
        // http://mt1.google.com/vt/lyrs=m
        QString server  = "mt";
        QString request = "vt";
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecGoogleWords(x, y, sec1, sec2);
        return QString("http://%1%2.google.com/%3/lyrs=%4&hl=%5&x=%6%7&y=%8&z=%9&s=%10").arg(server).arg(_getServerNum(x, y, 4)).arg(request).arg(_versionGoogleMap).arg(_language).arg(x).arg(sec1).arg(y).arg(zoom).arg(sec2);
    }
    break;
    case 2:
    {
        // http://mt1.google.com/vt/lyrs=s
        QString server  = "khm";
        QString request = "kh";
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecGoogleWords(x, y, sec1, sec2);
        return QString("http://%1%2.google.com/%3/v=%4&hl=%5&x=%6%7&y=%8&z=%9&s=%10").arg(server).arg(_getServerNum(x, y, 4)).arg(request).arg(_versionGoogleSatellite).arg(_language).arg(x).arg(sec1).arg(y).arg(zoom).arg(sec2);
    }
    break;
    case 3:
    {
        QString server  = "mts";
        QString request = "vt";
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecGoogleWords(x, y, sec1, sec2);
        return QString("http://%1%2.google.com/%3/lyrs=%4&hl=%5&x=%6%7&y=%8&z=%9&s=%10").arg(server).arg(_getServerNum(x, y, 4)).arg(request).arg(_versionGoogleLabels).arg(_language).arg(x).arg(sec1).arg(y).arg(zoom).arg(sec2);
    }
    break;
    case 4:
    {
        QString server  = "mt";
        QString request = "vt";
        QString sec1    = ""; // after &x=...
        QString sec2    = ""; // after &zoom=...
        _getSecGoogleWords(x, y, sec1, sec2);
        return QString("http://%1%2.google.com/%3/v=%4&hl=%5&x=%6%7&y=%8&z=%9&s=%10").arg(server).arg(_getServerNum(x, y, 4)).arg(request).arg(_versionGoogleTerrain).arg(_language).arg(x).arg(sec1).arg(y).arg(zoom).arg(sec2);
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
    QString html = QString(_googleReply->readAll());

    QRegExp reg("\"*https?://mt\\D?\\d..*/vt\\?lyrs=m@(\\d*)", Qt::CaseInsensitive);
    if (reg.indexIn(html) != -1) {
        QStringList gc = reg.capturedTexts();
        _versionGoogleMap = QString("m@%1").arg(gc[1]);
    }
    reg = QRegExp("\"*https?://khm\\D?\\d.googleapis.com/kh\\?v=(\\d*)", Qt::CaseInsensitive);
    if (reg.indexIn(html) != -1) {
        QStringList gc = reg.capturedTexts();
        _versionGoogleSatellite = gc[1];
    }
    reg = QRegExp("\"*https?://mt\\D?\\d..*/vt\\?lyrs=t@(\\d*),r@(\\d*)", Qt::CaseInsensitive);
    if (reg.indexIn(html) != -1) {
        QStringList gc = reg.capturedTexts();
        _versionGoogleTerrain = QString("t@%1,r@%2").arg(gc[1]).arg(gc[2]);
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
        QSslConfiguration conf = qheader.sslConfiguration();
        conf.setPeerVerifyMode(QSslSocket::VerifyNone);
        qheader.setSslConfiguration(conf);
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
