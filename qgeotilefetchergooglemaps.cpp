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
  m_tileSize(tileSize)
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
    _versionGoogleMap            = "m@336";
    _versionGoogleSatellite      = "194";
    _versionGoogleLabels         = "h@336";
    _versionGoogleTerrain        = "t@132,r@336";
    _secGoogleWord               = "Galileo";

    _tryCorrectGoogleVersions(m_networkManager);

    netRequest.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    netRequest.setRawHeader("Referrer", "http://maps.google.com/");
    netRequest.setRawHeader("Accept", "*/*");
    netRequest.setRawHeader("User-Agent", _userAgent);

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
    QString surl = _getURL(spec.mapId(), spec.x(), spec.y(), spec.zoom());
    QUrl url(surl);
    //qDebug() << "maps url:" << url;
    netRequest.setUrl(url); // The extra pair of parens disambiguates this from a function declaration

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
        //_tryCorrectGoogleVersions(networkManager);
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
        //_tryCorrectGoogleVersions(networkManager);
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
        //_tryCorrectGoogleVersions(networkManager);
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
        //_tryCorrectGoogleVersions(networkManager);
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
    QRegExp reg("\"*http://mt0.google.com/vt/lyrs=m@(\\d*)",Qt::CaseInsensitive);
    if (reg.indexIn(html) != -1) {
        QStringList gc = reg.capturedTexts();
        _versionGoogleMap = QString("m@%1").arg(gc[1]);
    }
    reg = QRegExp("\"*http://mt0.google.com/vt/lyrs=h@(\\d*)",Qt::CaseInsensitive);
    if (reg.indexIn(html) != -1) {
        QStringList gc = reg.capturedTexts();
        _versionGoogleLabels = QString("h@%1").arg(gc[1]);
    }
    reg = QRegExp("\"*http://khm\\D?\\d.google.com/kh/v=(\\d*)",Qt::CaseInsensitive);
    if (reg.indexIn(html) != -1) {
        QStringList gc = reg.capturedTexts();
        _versionGoogleSatellite = gc[1];
    }
    reg = QRegExp("\"*http://mt0.google.com/vt/lyrs=t@(\\d*),r@(\\d*)",Qt::CaseInsensitive);
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
        tProxy.setType(QNetworkProxy::NoProxy);
        networkManager->setProxy(tProxy);
        QString url = "http://maps.google.com/maps";
        qheader.setUrl(QUrl(url));
        QByteArray ua;
        ua.append(_userAgent);
        qheader.setRawHeader("User-Agent", ua);
        _googleReply = networkManager->get(qheader);
        connect(_googleReply, &QNetworkReply::finished, this, &QGeoTileFetcherGooglemaps::_googleVersionCompleted);
        connect(_googleReply, &QNetworkReply::destroyed, this, &QGeoTileFetcherGooglemaps::_replyDestroyed);
        connect(_googleReply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                this, &QGeoTileFetcherGooglemaps::_networkReplyError);
        networkManager->setProxy(proxy);
    }
}

QT_END_NAMESPACE
