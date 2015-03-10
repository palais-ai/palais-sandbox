#-------------------------------------------------
#
# Project created by QtCreator 2014-12-23T01:44:30
#
#-------------------------------------------------

QT       -= core gui

TARGET = ailib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += SMASTAR_LIBRARY

SOURCES += \
    Scheduler.cpp \
    Task.cpp \
    Graph.cpp \
    BehaviorTree.cpp

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
    Task.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
