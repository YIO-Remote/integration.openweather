# Plugin VERSION
OW_VERSION = $$system(git describe --match "v[0-9]*" --tags HEAD --always)
DEFINES += PLUGIN_VERSION=\\\"$$OW_VERSION\\\"

TEMPLATE  = lib
CONFIG   += c++14 plugin
QT       += network core quick
DEFINES  += DEBUG

INTG_LIB_PATH = $$(YIO_SRC)
isEmpty(INTG_LIB_PATH) {
    INTG_LIB_PATH = $$clean_path($$PWD/../integrations.library)
    message("Environment variables YIO_SRC not defined! Using '$$INTG_LIB_PATH' for integrations.library project.")
} else {
    INTG_LIB_PATH = $$(YIO_SRC)/integrations.library
    message("YIO_SRC is set: using '$$INTG_LIB_PATH' for integrations.library project.")
}

! include($$INTG_LIB_PATH/qmake-destination-path.pri) {
    error( "Couldn't find the qmake-destination-path.pri file!" )
}

! include($$INTG_LIB_PATH/yio-plugin-lib.pri) {
    error( "Cannot find the yio-plugin-lib.pri file!" )
}

QMAKE_SUBSTITUTES += openweather.json.in
# output path must be included for the output file from QMAKE_SUBSTITUTES
INCLUDEPATH += $$OUT_PWD
HEADERS  += \
    src/OpenWeather.h \
    src/ImageCache.h \
    src/WeatherModel.h
SOURCES  += \
    src/OpenWeather.cpp \
    src/ImageCache.cpp \
    src/WeatherModel.cpp

TARGET    = openweather

# Configure destination path. DESTDIR is set in qmake-destination-path.pri
DESTDIR = $$DESTDIR/plugins
OBJECTS_DIR = $$PWD/build/$$DESTINATION_PATH/obj
MOC_DIR = $$PWD/build/$$DESTINATION_PATH/moc
RCC_DIR = $$PWD/build/$$DESTINATION_PATH/qrc
UI_DIR = $$PWD/build/$$DESTINATION_PATH/ui

#DISTFILES += openweather.json

# install
unix {
    target.path = /usr/lib
    INSTALLS += target
}

