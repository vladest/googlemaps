# Google Maps plugin for QtLocation
GoogleMaps plugin for QtLocation module

Build instructions:
1. </your/Qt/folder/bin/>qmake -r
2. make
3. make install

Running:
Plugin parameter mustbe provided in order to make plugin works

PluginParameter {
    name: "googlemaps.useragent"
    value: "mygreatapp"
}

PluginParameter {
    name: "googlemaps.cachefolder"
    value: "/gmaps_cache"
}

PluginParameter {
    name: "googlemaps.route.apikey"
    value: "bla-bla"
}

PluginParameter {
    name: "googlemaps.maps.apikey"
    value: "bla-bla1"
}

PluginParameter {
    name: "googlemaps.geocode.apikey"
    value: "bla-bla2"
}

PluginParameter {
    name: "googlemaps.maps.tilesize"
    value: "256"
}

Debugging:
to check what plugins are loaded, set QT_DEBUG_PLUGINS=1 environment variable
