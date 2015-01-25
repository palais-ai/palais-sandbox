#-------------------------------------------------
#
# Project created by QtCreator 2015-01-05T14:49:26
#
#-------------------------------------------------

QT       += script quick

QT       -= gui

TARGET = sandboxcore
TEMPLATE = lib

DEFINES += LIBSANDBOXCORE_LIBRARY
macx:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Plugins/

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

SOURCES += \
    actor.cpp \
    javascriptbindings.cpp \
    knowledgemodel.cpp \
    ogrehelper.cpp \
    scene.cpp \
    DebugDrawer.cpp \
    scenedynamics.cpp

HEADERS +=\
    libsandboxcore_global.h \
    actor.h \
    javascriptbindings.h \
    knowledgemodel.h \
    ogrehelper.h \
    scene.h \
    DebugDrawer.h \
    scenedynamics.h

include(../linkOgreSDK.pri)
include(../linkBulletSDK.pri)

CONFIG(release, debug|release) {
    M_BUILD_DIR = release
} else {
    M_BUILD_DIR = debug
}

INCLUDEPATH += ../sandbox/

win32 {
    QMAKE_LIBDIR += $$OUT_PWD/../libmeshmagick/$$M_BUILD_DIR
}

unix|win32: LIBS += -L$$OUT_PWD/../libmeshmagick/ -lmeshmagick

INCLUDEPATH += $$PWD/../libmeshmagick/include
DEPENDPATH += $$PWD/../libmeshmagick

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libmeshmagick/$$M_BUILD_DIR/meshmagick.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libmeshmagick/libmeshmagick.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libqmlogre/release/ -lqmlogre
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libqmlogre/debug/ -lqmlogre
else:unix: LIBS += -L$$OUT_PWD/../libqmlogre/ -lqmlogre

INCLUDEPATH += $$PWD/../libqmlogre
DEPENDPATH += $$PWD/../libqmlogre
