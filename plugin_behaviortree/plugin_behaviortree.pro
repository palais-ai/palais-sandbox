#-------------------------------------------------
#
# Project created by QtCreator 2015-01-04T00:23:47
#
#-------------------------------------------------

QT += core script
QT -= gui

TARGET = plugin_behaviortree
TEMPLATE = lib
CONFIG += plugin

# QMAKE_CXXFLAGS += -save-temps

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

SOURCES += \
    Behavior.cpp \
    BehaviorPlugin.cpp \
    BehaviorPrototypes.cpp

HEADERS += \
    Behavior.h \
    BehaviorPlugin.h \
    BehaviorPrototypes.h

include(../linkOgreSDK.pri)

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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../ailib/LinearMath/release/ -lLinearMath
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../ailib/LinearMath/debug/ -lLinearMath
else:unix: LIBS += -L$$OUT_PWD/../ailib/LinearMath/ -lLinearMath

INCLUDEPATH += $$PWD/../ailib/LinearMath
DEPENDPATH += $$PWD/../ailib/LinearMath

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ailib/LinearMath/release/libLinearMath.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ailib/LinearMath/debug/libLinearMath.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ailib/LinearMath/release/LinearMath.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../ailib/LinearMath/debug/LinearMath.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../ailib/LinearMath/libLinearMath.a
