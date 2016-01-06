#ifndef QGEOCODEREPLYGOOGLEMAPS_H
#define QGEOCODEREPLYGOOGLEMAPS_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoCodeReply>

class QGeoCodeReplyGooglemaps : public QGeoCodeReply
{
    Q_OBJECT

public:
    explicit QGeoCodeReplyGooglemaps(QNetworkReply *reply, QObject *parent = 0);
    ~QGeoCodeReplyGooglemaps();

    void abort();

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    QNetworkReply *m_reply;
};

#endif // QGEOCODEREPLYORS_H
