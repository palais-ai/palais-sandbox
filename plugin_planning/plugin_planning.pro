#-------------------------------------------------
#
# Project created by QtCreator 2015-01-04T00:23:47
#
#-------------------------------------------------

QT += core script
QT -= gui

TARGET = plugin_planning
TEMPLATE = lib
CONFIG += plugin

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

SOURCES += \
    PlanningPlugin.cpp \
    Planning.cpp

HEADERS += \
    PlanningPlugin.h \
    Planning.h

include(../linkOgreSDK.pri)

CONFIG(release, debug|release) {
    M_BUILD_DIR = release
} else {
    M_BUILD_DIR = debug
}

win32 {
    QMAKE_LIBDIR += $$OUT_PWD/../ailib/AICore/$$M_BUILD_DIR
    QMAKE_LIBDIR += $$OUT_PWD/../ailib/LinearMath/$$M_BUILD_DIR
}

INCLUDEPATH += ../sandbox/

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libsandboxcore/release/ -lsandboxcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libsandboxcore/debug/ -lsandboxcore
else:unix: LIBS += -L$$OUT_PWD/../libsandboxcore/ -lsandboxcore

INCLUDEPATH += $$PWD/../libsandboxcore
DEPENDPATH += $$PWD/../libsandboxcore


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ailib/AICore/release/ -lailib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ailib/AICore/debug/ -lailib
else:unix: LIBS += -L$$OUT_PWD/../ailib/AICore/ -lailib

INCLUDEPATH += $$PWD/../ailib/AICore
DEPENDPATH += $$PWD/../ailib/AICore

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ailib/AICore/release/libailib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ailib/AICore/debug/libailib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ailib/AICore/release/ailib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ailib/AICore/debug/ailib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../ailib/AICore/libailib.a
