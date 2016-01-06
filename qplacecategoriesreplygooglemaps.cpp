#include "qplacecategoriesreplygooglemaps.h"

QPlaceCategoriesReplyGooglemaps::QPlaceCategoriesReplyGooglemaps(QObject *parent)
:   QPlaceReply(parent)
{
}

QPlaceCategoriesReplyGooglemaps::~QPlaceCategoriesReplyGooglemaps()
{
}

void QPlaceCategoriesReplyGooglemaps::emitFinished()
{
    setFinished(true);
    emit finished();
}

void QPlaceCategoriesReplyGooglemaps::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);
    emit error(errorCode, errorString);
}
