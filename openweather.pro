TEMPLATE    = lib
CONFIG      += plugin
QT          += network core quick
DEFINES     += DEBUG

include(../remote-software/qmake-target-platform.pri)
include(../remote-software/qmake-destination-path.pri)

HEADERS         = OpenWeather.h ImageCache.h WeatherModel.h \
                  ../remote-software/sources/entities/weatherinterface.h \
                  ../remote-software/sources/integrations/integration.h \
                  ../remote-software/sources/integrations/plugininterface.h

SOURCES         = OpenWeather.cpp ImageCache.cpp WeatherModel.cpp

TARGET          = openweather

# Configure destination path by "Operating System/Compiler/Processor Architecture/Build Configuration"
DESTDIR = $$PWD/../binaries/$$DESTINATION_PATH/plugins
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

