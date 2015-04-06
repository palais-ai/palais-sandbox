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
    Actor.cpp \
    Bindings/JavascriptBindings.cpp \
    KnowledgeModel.cpp \
    OgreHelper.cpp \
    Scene.cpp \
    DebugDrawer.cpp \
   # SceneDynamics.cpp \
    Bindings/ScriptTimer.cpp

HEADERS +=\
    libsandboxcore_global.h \
    Bindings/JavascriptBindings.h \
    KnowledgeModel.h \
    OgreHelper.h \
    Scene.h \
    DebugDrawer.h \
   # SceneDynamics.h \
    Bindings/ScriptTimer.h \
    Actor.h


include(../QOgre/linkOgreSDK.pri)
#include(../linkBulletSDK.pri)

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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QOgre/release/ -lQOgre
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QOgre/debug/ -lQOgre
else:unix: LIBS += -L$$OUT_PWD/../QOgre/ -lQOgre

INCLUDEPATH += $$PWD/../QOgre
DEPENDPATH += $$PWD/../QOgre
