QT       -= core gui
TEMPLATE = lib
CONFIG += staticlib
TARGET = dotsceneloader

SOURCES += DotSceneLoader.cpp \
    tinystr.cpp \
    tinyxml.cpp \
    tinyxmlerror.cpp \
    tinyxmlparser.cpp

HEADERS += DotSceneLoader.h \
    tinystr.h \
    tinyxml.h

# Copy all headers to build folder
Headers.path = $$OUT_PWD/include
Headers.files = $$files(*.h)
INSTALLS += Headers

include(../linkOgreSDK.pri)
