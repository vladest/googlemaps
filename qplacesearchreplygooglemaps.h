#ifndef QPLACESEARCHREPLYGOOGLEMAPS_H
#define QPLACESEARCHREPLYGOOGLEMAPS_H

#include <QtLocation/QPlaceSearchReply>

QT_BEGIN_NAMESPACE

class QNetworkReply;
class QPlaceManagerEngineGooglemaps;
class QPlaceResult;

class QPlaceSearchReplyGooglemaps : public QPlaceSearchReply
{
    Q_OBJECT

public:
    QPlaceSearchReplyGooglemaps(const QPlaceSearchRequest &request, QNetworkReply *reply,
                          QPlaceManagerEngineGooglemaps *parent);
    ~QPlaceSearchReplyGooglemaps();

    void abort();

private slots:
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
    void replyFinished();

private:
    QPlaceResult parsePlaceResult(const QJsonObject &item) const;

    QNetworkReply *m_reply;
};

QT_END_NAMESPACE

#endif // QPLACESEARCHREPLYORS_H
