CONFIG += qt
QT += qml quick
TEMPLATE = lib
TARGET = qmlogre

#CONFIG += c++11
CONFIG += staticlib

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

include(../linkOgreSDK.pri)

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

SOURCES += ogreitem.cpp \
    ogrenode.cpp \
    ogrecamerawrapper.cpp \
    cameranodeobject.cpp \
    ogreengine.cpp

HEADERS += \
    ogreitem.h \
    ogrenode.h \
    ogrecamerawrapper.h \
    cameranodeobject.h \
    ogreengine.h

# Copy all headers to build folder
Headers.path = $$OUT_PWD/include
Headers.files = $$files(*.h)
INSTALLS += Headers
