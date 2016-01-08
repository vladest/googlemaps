#ifndef QGEOMAPREPLYGOOGLEMAPS_H
#define QGEOMAPREPLYGOOGLEMAPS_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/private/qgeotilespec_p.h>
#include <QtLocation/private/qgeotiledmapreply_p.h>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QGeoMapReplyGooglemaps : public QGeoTiledMapReply
{
    Q_OBJECT

public:
    QGeoMapReplyGooglemaps(QNetworkReply *reply, const QGeoTileSpec &spec, QObject *parent = 0);
    ~QGeoMapReplyGooglemaps();

    void abort();

    QNetworkReply *networkReply() const;

private Q_SLOTS:
    void networkFinished();
    void networkError(QNetworkReply::NetworkError error);

private:
    QPointer<QNetworkReply> m_reply;
};

QT_END_NAMESPACE

#endif
