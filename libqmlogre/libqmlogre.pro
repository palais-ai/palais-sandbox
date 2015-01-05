CONFIG += qt
QT += qml quick
TEMPLATE = lib
TARGET = qmlogre

DEFINES += QMLOGRE_LIBRARY
macx:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Plugins/

#CONFIG += c++11

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

include(../linkOgreSDK.pri)

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

SOURCES += ogreitem.cpp \
    ogrenode.cpp \
    cameranodeobject.cpp \
    ogreengine.cpp

HEADERS += \
    ogreitem.h \
    ogrenode.h \
    cameranodeobject.h \
    ogreengine.h \
    qmlogre_global.h

# Copy all headers to build folder
Headers.path = $$OUT_PWD/include
Headers.files = $$files(*.h)
INSTALLS += Headers
