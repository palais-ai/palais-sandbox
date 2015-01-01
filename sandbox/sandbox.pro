CONFIG += qt
QT += qml quick script
TEMPLATE = app
TARGET = sandbox

#CONFIG += c++11

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

# This is important for win32 linkage.
DEFINES -= QJSONRPC_BUILD
DEFINES += QJSONRPC_SHARED

SOURCES += main.cpp \
    application.cpp \
    scenemanager.cpp \
    sceneloader.cpp \
    scene.cpp \
    projectmanager.cpp \
    actor.cpp \
    javascriptbindings.cpp \
    knowledgeservice.cpp \
    actorservice.cpp \
    models/actormodel.cpp \
    models/consolemodel.cpp \
    models/inspectormodel.cpp \
    utility/timedlogger.cpp \
    utility/DebugDrawer.cpp \
    utility/ogrehelper.cpp \
    utility/fighterplanner.cpp \
    utility/loghandler.cpp \
    models/scenemodel.cpp

HEADERS += \
    application.h \
    scenemanager.h \
    sceneloader.h \
    scene.h \
    projectmanager.h \
    actor.h \
    javascriptbindings.h \
    knowledgeservice.h \
    actorservice.h \
    models/actormodel.h \
    models/consolemodel.h \
    models/inspectormodel.h \
    utility/DebugDrawer.h \
    utility/fighterplanner.h \
    utility/ogrehelper.h \
    utility/timedlogger.h \
    utility/loghandler.h \
    models/scenemodel.h

OTHER_FILES += \
    config/resources.cfg

CONFIG(release, debug|release) {
    M_BUILD_DIR = release
} else {
    M_BUILD_DIR = debug
}

include(../linkOgreSDK.pri)

macx {
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error($$TARGET needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        message(Packaging Ogre libraries in $$OGREDIR)

        FrameworkFiles.files += $$OGREDIR/lib/release/Ogre.framework
        FrameworkFiles.path = Contents/Frameworks

        ConfigFiles.files += config/resources.cfg
        ConfigFiles.path = Contents/MacOS

        PluginFiles.files += $$OGREDIR/lib/RenderSystem_GL.dylib $$OGREDIR/lib/Plugin_OctreeSceneManager.dylib $$OUT_PWD/../libqjsonrpc/src/libqjsonrpc.1.dylib
        PluginFiles.path = Contents/Plugins

        MediaFiles.files += $$files(media/*)
        MediaFiles.path = Contents/MacOS/Resources

        QMAKE_BUNDLE_DATA += FrameworkFiles ConfigFiles PluginFiles MediaFiles
    }
} else:unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE
} else:win32 {
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error($$TARGET needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        message(Packaging Ogre libraries in $$OGREDIR)

        DESTDIR = $$OUT_PWD/$$M_BUILD_DIR

        message(Putting libraries in $$DESTDIR)
        message(QDIRs are in $$[QT_INSTALL_LIBS])

        package.path = $$DESTDIR
        package.files += $$OUT_PWD/../libqjsonrpc/src/$$M_BUILD_DIR/qjsonrpc1.dll
        package.files += $$OGREDIR/bin/release/opt/*.dll $$OGREDIR/bin/release/*.dll
        package.files += $$OGREDIR/bin/debug/opt/*.dll $$OGREDIR/bin/debug/*.dll
        package.files += $$[QT_INSTALL_LIBS]/../bin/*.dll
        package.CONFIG = no_check_exist

        # Copy all resources to build folder
        Resources.path = $$DESTDIR/Resources
        Resources.files += media/*

        # Copy all config files to build folder
        Config.path = $$DESTDIR
        Config.files += config/resources.cfg
        Config.files += config/$$M_BUILD_DIR/plugins.cfg

        INSTALLS += package Resources Config
    }
}

RESOURCES += resources/resources.qrc

win32 {
    QMAKE_LIBDIR += $$OUT_PWD/../libqmlogre/$$M_BUILD_DIR
    QMAKE_LIBDIR += $$OUT_PWD/../libdotsceneloader/$$M_BUILD_DIR
    QMAKE_LIBDIR += $$OUT_PWD/../libqjsonrpc/src/$$M_BUILD_DIR
    QMAKE_LIBDIR += $$OUT_PWD/../libmeshmagick/$$M_BUILD_DIR
}

unix|win32: LIBS += -L$$OUT_PWD/../libqmlogre/ -lqmlogre

INCLUDEPATH += $$PWD/../libqmlogre
DEPENDPATH += $$PWD/../libqmlogre

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libqmlogre/$$M_BUILD_DIR/qmlogre.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libqmlogre/libqmlogre.a

unix|win32: LIBS += -L$$OUT_PWD/../libdotsceneloader/ -ldotsceneloader

INCLUDEPATH += $$PWD/../libdotsceneloader
DEPENDPATH += $$PWD/../libdotsceneloader

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libdotsceneloader/$$M_BUILD_DIR/dotsceneloader.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libdotsceneloader/libdotsceneloader.a

unix|win32: LIBS += -L$$OUT_PWD/../libmeshmagick/ -lmeshmagick

INCLUDEPATH += $$PWD/../libmeshmagick/include
DEPENDPATH += $$PWD/../libmeshmagick

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libmeshmagick/$$M_BUILD_DIR/meshmagick.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libmeshmagick/libmeshmagick.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libqjsonrpc/src/release/ -lqjsonrpc1
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libqjsonrpc/src/debug/ -lqjsonrpc1
else:unix: LIBS += -L$$OUT_PWD/../libqjsonrpc/src/ -lqjsonrpc

INCLUDEPATH += $$PWD/../libqjsonrpc/src
DEPENDPATH += $$PWD/../libqjsonrpc/src

INCLUDEPATH += ../../smastar/
INCLUDEPATH += $$PWD
