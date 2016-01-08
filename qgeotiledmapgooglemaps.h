#ifndef QGEOMAPGOOGLEMAPS_H
#define QGEOMAPGOOGLEMAPS_H

#include "QtLocation/private/qgeotiledmap_p.h"
#include <QtGui/QImage>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QGeoTiledMappingManagerEngineGooglemaps;

class QGeoTiledMapGooglemaps: public QGeoTiledMap
{
Q_OBJECT
public:
    QGeoTiledMapGooglemaps(QGeoTiledMappingManagerEngineGooglemaps *engine, QObject *parent = 0);
    ~QGeoTiledMapGooglemaps();

    QString getViewCopyright();
    void evaluateCopyrights(const QSet<QGeoTileSpec> &visibleTiles);

private:
    //QImage m_logo;
    QImage m_copyrightsSlab;
    QString m_lastCopyrightsString;
    QPointer<QGeoTiledMappingManagerEngineGooglemaps> m_engine;

    Q_DISABLE_COPY(QGeoTiledMapGooglemaps)
};

QT_END_NAMESPACE

#endif // QGEOMAPGOOGLEMAPS_H
