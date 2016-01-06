#ifndef QPLACECATEGORIESREPLYGOOGLEMAPS_H
#define QPLACECATEGORIESREPLYGOOGLEMAPS_H

#include <QtLocation/QPlaceReply>

class QPlaceCategoriesReplyGooglemaps : public QPlaceReply
{
    Q_OBJECT

public:
    explicit QPlaceCategoriesReplyGooglemaps(QObject *parent = 0);
    ~QPlaceCategoriesReplyGooglemaps();

    void emitFinished();
    void setError(QPlaceReply::Error errorCode, const QString &errorString);
};

#endif // QPLACECATEGORIESREPLYGOOGLEMAPS_H
