TEMPLATE    = lib
CONFIG      += plugin
QT          += network core quick
DEFINES     += DEBUG

include(../remote-software/qmake-target-platform.pri)
include(../remote-software/qmake-destination-path.pri)

HEADERS         = OpenWeather.h RestClient.h ImageCache.h \
                  ../remote-software/sources/integrations/integration.h \
                  ../remote-software/sources/integrations/integrationinterface.h

SOURCES         = OpenWeather.cpp RestClient.cpp ImageCache.cpp

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
