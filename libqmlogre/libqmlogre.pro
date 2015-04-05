CONFIG += qt
QT += qml quick
TEMPLATE = lib
TARGET = qmlogre

DEFINES += QMLOGRE_LIBRARY
macx:QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Plugins/

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

#CONFIG += c++11

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

include(linkOgreSDK.pri)

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

SOURCES += \
    QOCamera.cpp \
    QONode.cpp \
    QOEngine.cpp \
    QOItem.cpp

HEADERS += \
    qmlogre_global.h \
    QOCamera.h \
    QONode.h \
    QOItem.h \
    QOEngine.h

# Copy all headers to build folder
Headers.path = $$OUT_PWD/include
Headers.files = $$files(*.h)
INSTALLS += Headers
