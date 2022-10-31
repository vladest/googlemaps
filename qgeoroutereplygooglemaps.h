#ifndef QGEOROUTEREPLYORS_H
#define QGEOROUTEREPLYORS_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QGeoRouteReply>

QT_BEGIN_NAMESPACE

class QGeoRouteReplyGooglemaps : public QGeoRouteReply
{
    Q_OBJECT

public:
//    explicit QGeoRouteReplyGooglemaps(QObject *parent = 0);
    QGeoRouteReplyGooglemaps(QNetworkReply *reply, const QGeoRouteRequest &request, QObject *parent = 0);
    ~QGeoRouteReplyGooglemaps();

    void abort() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void networkReplyFinished();
    void networkReplyError(QNetworkReply::NetworkError error);

private:
    QNetworkReply *m_reply;
};

QT_END_NAMESPACE

#endif // QGEOROUTEREPLYOrs_H

