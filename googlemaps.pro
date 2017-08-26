TARGET = qtgeoservices_googlemaps

qtHaveModule(location-private) {
	QT += location-private
} else {
	QT += location
}
qtHaveModule(positioning-private) {
	QT += positioning-private
} else {
	QT += positioning
}
QT += network
INCLUDEPATH += ../ ./

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = QGeoServiceProviderFactoryGooglemaps
load(qt_plugin)

HEADERS += \
    qgeoserviceproviderplugingooglemaps.h \
    qgeocodingmanagerenginegooglemaps.h \
    qgeocodereplygooglemaps.h \
    qgeoroutingmanagerenginegooglemaps.h \
    qgeoroutereplygooglemaps.h \
    qplacemanagerenginegooglemaps.h \
    qplacesearchreplygooglemaps.h \
    qplacecategoriesreplygooglemaps.h \
    qgeomapreplygooglemaps.h \
    qgeotiledmapgooglemaps.h \
    qgeotiledmappingmanagerenginegooglemaps.h \
    qgeotilefetchergooglemaps.h \
    qplacesearchsuggestionreplyimpl.h \
    qgeoerror_messages.h

SOURCES += \
    qgeoserviceproviderplugingooglemaps.cpp \
    qgeocodingmanagerenginegooglemaps.cpp \
    qgeocodereplygooglemaps.cpp \
    qgeoroutingmanagerenginegooglemaps.cpp \
    qgeoroutereplygooglemaps.cpp \
    qplacemanagerenginegooglemaps.cpp \
    qplacesearchreplygooglemaps.cpp \
    qplacecategoriesreplygooglemaps.cpp \
    qgeomapreplygooglemaps.cpp \
    qgeotiledmapgooglemaps.cpp \
    qgeotiledmappingmanagerenginegooglemaps.cpp \
    qgeotilefetchergooglemaps.cpp \
    qplacesearchsuggestionreplyimpl.cpp \
    qgeoerror_messages.cpp


OTHER_FILES += \
    googlemaps_plugin.json

