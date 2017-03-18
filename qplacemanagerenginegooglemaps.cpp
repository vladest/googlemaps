#include "qplacemanagerenginegooglemaps.h"
#include "qplacesearchreplygooglemaps.h"
#include "qplacecategoriesreplygooglemaps.h"
#include "qplacesearchsuggestionreplyimpl.h"

#include <QtCore/QUrlQuery>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QRegularExpression>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCircle>
#include <QtLocation/private/unsupportedreplies_p.h>

#include <QtCore/QElapsedTimer>

namespace
{
QString SpecialPhrasesBaseUrl = QStringLiteral("http://wiki.openstreetmap.org/wiki/Special:Export/Nominatim/Special_Phrases/");

QString nameForTagKey(const QString &tagKey)
{
    if (tagKey == QLatin1String("aeroway"))
        return QPlaceManagerEngineGooglemaps::tr("Aeroway");
    else if (tagKey == QLatin1String("amenity"))
        return QPlaceManagerEngineGooglemaps::tr("Amenity");
    else if (tagKey == QLatin1String("building"))
        return QPlaceManagerEngineGooglemaps::tr("Building");
    else if (tagKey == QLatin1String("highway"))
        return QPlaceManagerEngineGooglemaps::tr("Highway");
    else if (tagKey == QLatin1String("historic"))
        return QPlaceManagerEngineGooglemaps::tr("Historic");
    else if (tagKey == QLatin1String("landuse"))
        return QPlaceManagerEngineGooglemaps::tr("Land use");
    else if (tagKey == QLatin1String("leisure"))
        return QPlaceManagerEngineGooglemaps::tr("Leisure");
    else if (tagKey == QLatin1String("man_made"))
        return QPlaceManagerEngineGooglemaps::tr("Man made");
    else if (tagKey == QLatin1String("natural"))
        return QPlaceManagerEngineGooglemaps::tr("Natural");
    else if (tagKey == QLatin1String("place"))
        return QPlaceManagerEngineGooglemaps::tr("Place");
    else if (tagKey == QLatin1String("railway"))
        return QPlaceManagerEngineGooglemaps::tr("Railway");
    else if (tagKey == QLatin1String("shop"))
        return QPlaceManagerEngineGooglemaps::tr("Shop");
    else if (tagKey == QLatin1String("tourism"))
        return QPlaceManagerEngineGooglemaps::tr("Tourism");
    else if (tagKey == QLatin1String("waterway"))
        return QPlaceManagerEngineGooglemaps::tr("Waterway");
    else
        return tagKey;
}

}


static bool addAtForBoundingArea(const QGeoShape &area,
                                 QUrlQuery *queryItems)
{
    QGeoCoordinate center = area.center();
    if (!center.isValid())
        return false;

    queryItems->addQueryItem(QStringLiteral("location"),
                             QString::number(center.latitude()) +
                             QLatin1Char(',') +
                             QString::number(center.longitude()));
    // for google maps the shape has to be a circle
    QGeoCircle* cirle = new QGeoCircle(area);
    if(cirle != NULL)
    {
        qreal radiusCorrected = cirle->radius();
        if(radiusCorrected < 1)
            radiusCorrected = 1;
        queryItems->addQueryItem(QStringLiteral("radius"),
                                 QString::number(radiusCorrected));
    }
    return true;
}

QPlaceManagerEngineGooglemaps::QPlaceManagerEngineGooglemaps(const QVariantMap &parameters,
                                               QGeoServiceProvider::Error *error,
                                               QString *errorString)
:   QPlaceManagerEngine(parameters), m_networkManager(new QNetworkAccessManager(this)),
    m_categoriesReply(0)
{
//    if (parameters.contains(QStringLiteral("ors.useragent")))
//        m_userAgent = parameters.value(QStringLiteral("ors.useragent")).toString().toLatin1();
//    else
//        m_userAgent = "Qt Location based application";

    if (parameters.contains(QStringLiteral("googlemaps.places.apikey")))
        m_apiKey = parameters.value(QStringLiteral("googlemaps.places.apikey")).toString().toLatin1();
    else
        m_apiKey = parameters.value(QStringLiteral("googlemaps.apikey")).toString().toLatin1();

//    if (parameters.contains(QStringLiteral("Ors.places.host")))
//        m_urlPrefix = parameters.value(QStringLiteral("Ors.places.host")).toString();
//    else
        m_urlPrefix = QStringLiteral("https://maps.googleapis.com/maps/api/place");

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QPlaceManagerEngineGooglemaps::~QPlaceManagerEngineGooglemaps()
{
}

QPlaceSearchReply *QPlaceManagerEngineGooglemaps::search(const QPlaceSearchRequest &request)
{
    bool unsupported = false;

    // Only public visibility supported
    unsupported |= request.visibilityScope() != QLocation::UnspecifiedVisibility &&
                   request.visibilityScope() != QLocation::PublicVisibility;
    unsupported |= request.searchTerm().isEmpty() && request.categories().isEmpty();

    if (unsupported)
        return QPlaceManagerEngine::search(request);

    QUrlQuery queryItems;

    queryItems.addQueryItem(QStringLiteral("format"), QStringLiteral("jsonv2"));

    //queryItems.addQueryItem(QStringLiteral("accept-language"), QStringLiteral("en"));

    QGeoRectangle boundingBox;
    QGeoShape searchArea = request.searchArea();
    switch (searchArea.type()) {
    case QGeoShape::CircleType: {
        QGeoCircle c(searchArea);
        qreal radius = c.radius();
        if (radius < 0)
            radius = 50000;

        boundingBox = QGeoRectangle(c.center().atDistanceAndAzimuth(radius, -45),
                                    c.center().atDistanceAndAzimuth(radius, 135));
        break;
    }
    case QGeoShape::RectangleType:
        boundingBox = searchArea;
        break;
    default:
        ;
    }

    if (!boundingBox.isEmpty()) {
        queryItems.addQueryItem(QStringLiteral("bounded"), QStringLiteral("1"));
        QString coordinates;
        coordinates = QString::number(boundingBox.topLeft().longitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.topLeft().latitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.bottomRight().longitude()) + QLatin1Char(',') +
                      QString::number(boundingBox.bottomRight().latitude());
        queryItems.addQueryItem(QStringLiteral("viewbox"), coordinates);
    }

    QStringList queryParts;
    if (!request.searchTerm().isEmpty())
        queryParts.append(request.searchTerm());

    foreach (const QPlaceCategory &category, request.categories()) {
        QString id = category.categoryId();
        int index = id.indexOf(QLatin1Char('='));
        if (index != -1)
            id = id.mid(index+1);
        queryParts.append(QLatin1Char('[') + id + QLatin1Char(']'));
    }

    queryItems.addQueryItem(QStringLiteral("q"), queryParts.join(QLatin1Char('+')));

    QVariantMap parameters = request.searchContext().toMap();

    QStringList placeIds = parameters.value(QStringLiteral("ExcludePlaceIds")).toStringList();
    if (!placeIds.isEmpty())
        queryItems.addQueryItem(QStringLiteral("exclude_place_ids"), placeIds.join(QLatin1Char(',')));

    queryItems.addQueryItem(QStringLiteral("addressdetails"), QStringLiteral("1"));

    QUrl requestUrl(m_urlPrefix);
    requestUrl.setQuery(queryItems);

    QNetworkReply *networkReply = m_networkManager->get(QNetworkRequest(requestUrl));

    QPlaceSearchReplyGooglemaps *reply = new QPlaceSearchReplyGooglemaps(request, networkReply, this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
            this, SLOT(replyError(QPlaceReply::Error,QString)));

    return reply;
}

QPlaceSearchSuggestionReply *QPlaceManagerEngineGooglemaps::searchSuggestions(const QPlaceSearchRequest &query)
{
    bool unsupported = false;

    unsupported |= query.visibilityScope() != QLocation::UnspecifiedVisibility &&
                   query.visibilityScope() != QLocation::PublicVisibility;

    unsupported |= !query.categories().isEmpty();
    unsupported |= !query.recommendationId().isEmpty();

    if (unsupported) {
        QPlaceSearchSuggestionReplyImpl *reply = new QPlaceSearchSuggestionReplyImpl(0, this);
        connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
        connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
                this, SLOT(replyError(QPlaceReply::Error,QString)));
        QMetaObject::invokeMethod(reply, "setError", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, QPlaceReply::BadArgumentError),
                                  Q_ARG(QString, "Unsupported search request options specified."));
        return reply;
    }

    QUrl requestUrl(m_urlPrefix + QStringLiteral("/autocomplete/json"));

    QUrlQuery queryItems;

    queryItems.addQueryItem(QStringLiteral("input"), query.searchTerm());

    if (!addAtForBoundingArea(query.searchArea(), &queryItems)) {
        QPlaceSearchSuggestionReplyImpl *reply = new QPlaceSearchSuggestionReplyImpl(0, this);
        connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
        connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
                this, SLOT(replyError(QPlaceReply::Error,QString)));
        QMetaObject::invokeMethod(reply, "setError", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, QPlaceReply::BadArgumentError),
                                  Q_ARG(QString, "Invalid search area provided"));
        return reply;
    }

    requestUrl.setQuery(queryItems);

    QNetworkReply *networkReply = sendRequest(requestUrl);

    QPlaceSearchSuggestionReplyImpl *reply = new QPlaceSearchSuggestionReplyImpl(networkReply, this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
            this, SLOT(replyError(QPlaceReply::Error,QString)));

    return reply;
}

QPlaceReply *QPlaceManagerEngineGooglemaps::initializeCategories()
{
    // Only fetch categories once
    if (m_categories.isEmpty() && !m_categoriesReply) {
        m_categoryLocales = m_locales;
        m_categoryLocales.append(QLocale(QLocale::English));
        fetchNextCategoryLocale();
    }

    QPlaceCategoriesReplyGooglemaps *reply = new QPlaceCategoriesReplyGooglemaps(this);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
    connect(reply, SIGNAL(error(QPlaceReply::Error,QString)),
            this, SLOT(replyError(QPlaceReply::Error,QString)));

    // TODO delayed finished() emission
    if (!m_categories.isEmpty())
        reply->emitFinished();

    m_pendingCategoriesReply.append(reply);
    return reply;
}

QString QPlaceManagerEngineGooglemaps::parentCategoryId(const QString &categoryId) const
{
    Q_UNUSED(categoryId)

    // Only a two category levels
    return QString();
}

QStringList QPlaceManagerEngineGooglemaps::childCategoryIds(const QString &categoryId) const
{
    return m_subcategories.value(categoryId);
}

QPlaceCategory QPlaceManagerEngineGooglemaps::category(const QString &categoryId) const
{
    return m_categories.value(categoryId);
}

QList<QPlaceCategory> QPlaceManagerEngineGooglemaps::childCategories(const QString &parentId) const
{
    QList<QPlaceCategory> categories;
    foreach (const QString &id, m_subcategories.value(parentId))
        categories.append(m_categories.value(id));
    return categories;
}

QList<QLocale> QPlaceManagerEngineGooglemaps::locales() const
{
    return m_locales;
}

void QPlaceManagerEngineGooglemaps::setLocales(const QList<QLocale> &locales)
{
    m_locales = locales;
}

void QPlaceManagerEngineGooglemaps::categoryReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    QXmlStreamReader parser(reply);
    while (!parser.atEnd() && parser.readNextStartElement()) {
        if (parser.name() == QLatin1String("mediawiki"))
            continue;
        if (parser.name() == QLatin1String("page"))
            continue;
        if (parser.name() == QLatin1String("revision"))
            continue;
        if (parser.name() == QLatin1String("text")) {
            // parse
            QString page = parser.readElementText();
            QRegularExpression regex(QStringLiteral("\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([^|]+) \\|\\| ([\\-YN])"));
            QRegularExpressionMatchIterator i = regex.globalMatch(page);
            while (i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                QString name = match.capturedRef(1).toString();
                QString tagKey = match.capturedRef(2).toString();
                QString tagValue = match.capturedRef(3).toString();
                QString op = match.capturedRef(4).toString();
                QString plural = match.capturedRef(5).toString();

                // Only interested in any operator plural forms
                if (op != QLatin1String("-") || plural != QLatin1String("Y"))
                    continue;

                if (!m_categories.contains(tagKey)) {
                    QPlaceCategory category;
                    category.setCategoryId(tagKey);
                    category.setName(nameForTagKey(tagKey));
                    m_categories.insert(category.categoryId(), category);
                    m_subcategories[QString()].append(tagKey);
                    emit categoryAdded(category, QString());
                }

                QPlaceCategory category;
                category.setCategoryId(tagKey + QLatin1Char('=') + tagValue);
                category.setName(name);

                if (!m_categories.contains(category.categoryId())) {
                    m_categories.insert(category.categoryId(), category);
                    m_subcategories[tagKey].append(category.categoryId());
                    emit categoryAdded(category, tagKey);
                }
            }
        }

        parser.skipCurrentElement();
    }

    if (m_categories.isEmpty() && !m_categoryLocales.isEmpty()) {
        fetchNextCategoryLocale();
        return;
    } else {
        m_categoryLocales.clear();
    }

    foreach (QPlaceCategoriesReplyGooglemaps *reply, m_pendingCategoriesReply)
        reply->emitFinished();
    m_pendingCategoriesReply.clear();
}

void QPlaceManagerEngineGooglemaps::categoryReplyError()
{
    foreach (QPlaceCategoriesReplyGooglemaps *reply, m_pendingCategoriesReply)
        reply->setError(QPlaceReply::CommunicationError, tr("Network request error"));
}

void QPlaceManagerEngineGooglemaps::replyFinished()
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit finished(reply);
}

void QPlaceManagerEngineGooglemaps::replyError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply *reply = qobject_cast<QPlaceReply *>(sender());
    if (reply)
        emit error(reply, errorCode, errorString);
}

void QPlaceManagerEngineGooglemaps::fetchNextCategoryLocale()
{
    if (m_categoryLocales.isEmpty()) {
        qWarning("No locales specified to fetch categories for");
        return;
    }

    QLocale locale = m_categoryLocales.takeFirst();

    // FIXME: Categories should be cached.
    QUrl requestUrl = QUrl(SpecialPhrasesBaseUrl + locale.name().left(2).toUpper());

    m_categoriesReply = m_networkManager->get(QNetworkRequest(requestUrl));
    connect(m_categoriesReply, SIGNAL(finished()), this, SLOT(categoryReplyFinished()));
    connect(m_categoriesReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(categoryReplyError()));
}


QNetworkReply *QPlaceManagerEngineGooglemaps::sendRequest(const QUrl &url)
{
    QUrlQuery queryItems(url);
    queryItems.addQueryItem(QStringLiteral("key"), m_apiKey);

    QUrl requestUrl = url;
    requestUrl.setQuery(queryItems);

    QNetworkRequest request;
    request.setUrl(requestUrl);

    request.setRawHeader("Accept", "application/json");
//    request.setRawHeader("Accept-Language", createLanguageString());

    return m_networkManager->get(request);
}
