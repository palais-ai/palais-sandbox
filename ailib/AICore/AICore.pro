#-------------------------------------------------
#
# Project created by QtCreator 2014-12-23T01:44:30
#
#-------------------------------------------------

QT       -= core gui

TARGET = ailib
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11

DEFINES += SMASTAR_LIBRARY

SOURCES += \
    Scheduler.cpp \
    Task.cpp \
    Graph.cpp \
    BehaviorTree.cpp \
    HighResolutionTime.cpp

HEADERS +=\
    ai_global.h \
    IDAStar.h \
    AStar.h \
    Graph.h \
    Any.h \
    Blackboard.h \
    GOAP.h \
    BehaviorTree.h \
    Scheduler.h \
    Task.h \
    HighResolutionTime.h


CONFIG(release, debug|release) {
    M_BUILD_DIR = release
} else {
    M_BUILD_DIR = debug
}

win32 {
    QMAKE_LIBDIR += $$OUT_PWD/../LinearMath/$$M_BUILD_DIR
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../LinearMath/$$M_BUILD_DIR/ -lLinearMath
else:unix: LIBS += -L$$OUT_PWD/../LinearMath/ -lLinearMath

INCLUDEPATH += $$PWD/../LinearMath
DEPENDPATH += $$PWD/../LinearMath

win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../LinearMath/$$M_BUILD_DIR/libLinearMath.a
else:win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../LinearMath/$$M_BUILD_DIR/LinearMath.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../LinearMath/libLinearMath.a
