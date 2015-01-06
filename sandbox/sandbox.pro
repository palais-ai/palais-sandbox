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

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

SOURCES += main.cpp \
    application.cpp \
    scenemanager.cpp \
    sceneloader.cpp \
    projectmanager.cpp \
    knowledgeservice.cpp \
    actorservice.cpp \
    models/actormodel.cpp \
    models/consolemodel.cpp \
    models/inspectormodel.cpp \
    utility/timedlogger.cpp \
    utility/loghandler.cpp \
    models/scenemodel.cpp \
    PluginManager.cpp

HEADERS += \
    application.h \
    scenemanager.h \
    sceneloader.h \
    projectmanager.h \
    knowledgeservice.h \
    actorservice.h \
    models/actormodel.h \
    models/consolemodel.h \
    models/inspectormodel.h \
    utility/timedlogger.h \
    utility/loghandler.h \
    models/scenemodel.h \
    PluginInterface.h \
    utility/MetatypeDeclarations.h \
    PluginManager.h

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

        PluginFiles.files += $$OGREDIR/lib/RenderSystem_GL.dylib
        PluginFiles.files += $$OGREDIR/lib/Plugin_OctreeSceneManager.dylib
        PluginFiles.files += $$OUT_PWD/../libqjsonrpc/src/libqjsonrpc.1.dylib
        PluginFiles.files += $$OUT_PWD/../libqmlogre/libqmlogre.1.dylib
        PluginFiles.files += $$OUT_PWD/../libsandboxcore/libsandboxcore.1.dylib
        PluginFiles.files += $$OUT_PWD/../plugin_pathfinding/libplugin_pathfinding.dylib
        PluginFiles.files += $$OUT_PWD/../plugin_planning/libplugin_planning.dylib
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
        package.files += $$OUT_PWD/../libqmlogre/$$M_BUILD_DIR/libqmlogre1.dll
        package.files += $$OUT_PWD/../libsandboxcore/$$M_BUILD_DIR/libsandboxcore1.dll
        package.files += $$OUT_PWD/../plugin_pathfinding/$$M_BUILD_DIR/libplugin_pathfinding.dll
        package.files += $$OUT_PWD/../plugin_planning/$$M_BUILD_DIR/libplugin_planning.dll
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

INCLUDEPATH += $$PWD

win32 {
    QMAKE_LIBDIR += $$OUT_PWD/../libdotsceneloader/$$M_BUILD_DIR
}

unix|win32: LIBS += -L$$OUT_PWD/../libdotsceneloader/ -ldotsceneloader

INCLUDEPATH += $$PWD/../libdotsceneloader
DEPENDPATH += $$PWD/../libdotsceneloader

win32:!win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libdotsceneloader/$$M_BUILD_DIR/dotsceneloader.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../libdotsceneloader/libdotsceneloader.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libqjsonrpc/src/release/ -lqjsonrpc1
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libqjsonrpc/src/debug/ -lqjsonrpc1
else:unix: LIBS += -L$$OUT_PWD/../libqjsonrpc/src/ -lqjsonrpc

INCLUDEPATH += $$PWD/../libqjsonrpc/src
DEPENDPATH += $$PWD/../libqjsonrpc/src

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libqmlogre/release/ -lqmlogre
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libqmlogre/debug/ -lqmlogre
else:unix: LIBS += -L$$OUT_PWD/../libqmlogre/ -lqmlogre

INCLUDEPATH += $$PWD/../libqmlogre
DEPENDPATH += $$PWD/../libqmlogre

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libsandboxcore/release/ -lsandboxcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libsandboxcore/debug/ -lsandboxcore
else:unix: LIBS += -L$$OUT_PWD/../libsandboxcore/ -lsandboxcore

INCLUDEPATH += $$PWD/../libsandboxcore
DEPENDPATH += $$PWD/../libsandboxcore

