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

INCLUDEPATH += ../sandbox/
INCLUDEPATH += ../ailib/

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libsandboxcore/release/ -lsandboxcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libsandboxcore/debug/ -lsandboxcore
else:unix: LIBS += -L$$OUT_PWD/../libsandboxcore/ -lsandboxcore

INCLUDEPATH += $$PWD/../libsandboxcore
DEPENDPATH += $$PWD/../libsandboxcore
