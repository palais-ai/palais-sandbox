CONFIG += qt
QT += qml quick script
TEMPLATE = app
TARGET = Palais

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

ICON = $$PWD/intermediary/Icon.icns
RC_FILE = $$PWD/intermediary/Palais.rc

SOURCES += main.cpp \
    Application.cpp \
    SceneManager.cpp \
    SceneLoader.cpp \
    ProjectManager.cpp \
    Models/ActorModel.cpp \
    Models/ConsoleModel.cpp \
    Models/InspectorModel.cpp \
    Utility/TimedLogger.cpp \
    Utility/LogHandler.cpp \
    Models/SceneModel.cpp \
    PluginManager.cpp

HEADERS += \
    Application.h \
    SceneManager.h \
    SceneLoader.h \
    ProjectManager.h \
    Models/ActorModel.h \
    Models/ConsoleModel.h \
    Models/InspectorModel.h \
    Utility/TimedLogger.h \
    Utility/LogHandler.h \
    Models/SceneModel.h \
    PluginInterface.h \
    Utility/MetatypeDeclarations.h \
    PluginManager.h

OTHER_FILES += \
    config/resources.cfg

CONFIG(release, debug|release) {
    M_BUILD_DIR = release
} else {
    M_BUILD_DIR = debug
}

include(../QOgre/linkOgreSDK.pri)

macx {
    BULLETDIR = $$(BULLET_HOME)
    isEmpty(BULLETDIR) {
        error($$TARGET needs bullet to be built. Please set the environment variable BULLET_HOME pointing to your bullet root directory.)
    }

    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error($$TARGET needs OGRE to be built. Please set the environment variable OGRE_HOME pointing to your OGRE root directory.)
    }

    message(Packaging Ogre libraries in $$OGREDIR)
    message(Packaging bullet libraries in $$BULLETDIR)
    message(Packaging Qt libraries in $$[QT_INSTALL_LIBS])

    FrameworkFiles.files += $$OGREDIR/lib/release/Ogre.framework

    MY_BUNDLE_DIR = $$OUT_PWD/$$basename(TARGET).app
    message($$MY_BUNDLE_DIR is the bundle dir)
    message($$PWD/resources is the qml dir)

    # Uncomment this line for deployment.
    #QMAKE_POST_LINK = macdeployqt $$MY_BUNDLE_DIR -qmldir=$$PWD/resources

    FrameworkFiles.path = Contents/Frameworks

    LicenseFiles.path = Contents/Licenses
    LicenseFiles.files += $$files($$PWD/licenses/*)

    ConfigFiles.files += config/resources.cfg
    ConfigFiles.path = Contents/MacOS


    !exists($$OGREDIR/lib/macosx) { # Ogre 1.8
        PluginFiles.files += $$OGREDIR/lib/RenderSystem_GL.dylib
        PluginFiles.files += $$OGREDIR/lib/Plugin_OctreeSceneManager.dylib
    } else { # Ogre 1.9
        PluginFiles.files += $$OGREDIR/lib/macosx/Release/RenderSystem_GL.dylib
        PluginFiles.files += $$OGREDIR/lib/macosx/Release/Plugin_OctreeSceneManager.dylib
    }

    PluginFiles.files += $$files($$BULLETDIR/src/BulletCollision/*.dylib)
    PluginFiles.files += $$files($$BULLETDIR/src/BulletDynamics/*.dylib)
    PluginFiles.files += $$files($$BULLETDIR/src/LinearMath/*.dylib)
    PluginFiles.files += $$OUT_PWD/../QOgre/libQOgre.1.dylib
    PluginFiles.files += $$OUT_PWD/../libsandboxcore/libsandboxcore.1.dylib
    PluginFiles.files += $$OUT_PWD/../plugin_pathfinding/libplugin_pathfinding.dylib
    PluginFiles.files += $$OUT_PWD/../plugin_planning/libplugin_planning.dylib
    PluginFiles.files += $$OUT_PWD/../plugin_behaviortree/libplugin_behaviortree.dylib
    PluginFiles.files += $$OUT_PWD/../plugin_genetic/libplugin_genetic.dylib
    PluginFiles.files += $$OUT_PWD/../plugin_steering/libplugin_steering.dylib
    PluginFiles.path = Contents/Plugins

    MediaFiles.files += $$files(media/*)
    MediaFiles.path = Contents/MacOS/Resources

    QMAKE_BUNDLE_DATA += FrameworkFiles ConfigFiles PluginFiles MediaFiles LicenseFiles
} else:unix {
    # TODO: Linux build / deployment
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE
} else:win32 {
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error($$TARGET needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    }

    message(Packaging Ogre libraries in $$OGREDIR)

    DESTDIR = $$OUT_PWD/$$M_BUILD_DIR

    message(Putting libraries in $$DESTDIR)
    message(QDIRs are in $$[QT_INSTALL_LIBS])

    plugins.path = $$DESTDIR/Plugins
    plugins.files += $$OUT_PWD/../plugin_pathfinding/$$M_BUILD_DIR/plugin_pathfinding.dll
    plugins.files += $$OUT_PWD/../plugin_planning/$$M_BUILD_DIR/plugin_planning.dll
    plugins.files += $$OUT_PWD/../plugin_behaviortree/$$M_BUILD_DIR/plugin_behaviortree.dll
    plugins.CONFIG = no_check_exist

    package.path = $$DESTDIR
    package.files += $$OUT_PWD/../QOgre/$$M_BUILD_DIR/QOgre.dll
    package.files += $$OUT_PWD/../libsandboxcore/$$M_BUILD_DIR/sandboxcore.dll

    OGRE_LIBS_SUFFIX =
    CONFIG(debug, debug|release) {
        OGRE_LIBS_SUFFIX = _d
    }

    OGRE_LIBS += OgreMain$$OGRE_LIBS_SUFFIX
    OGRE_LIBS += RenderSystem_GL$$OGRE_LIBS_SUFFIX
    OGRE_LIBS += Plugin_OctreeSceneManager$$OGRE_LIBS_SUFFIX

    for(the_lib, OGRE_LIBS):package.files += $$OGREDIR/bin/$$M_BUILD_DIR/$${the_lib}.dll
    package.CONFIG = no_check_exist

    MY_BUNDLE_DIR = $$DESTDIR/$$basename(TARGET).exe
    message($$MY_BUNDLE_DIR is the bundle dir)
    message($$PWD/resources is the qml dir)

    # Uncomment this line for deployment.
    #QMAKE_POST_LINK = $$[QT_INSTALL_LIBS]/../bin/windeployqt $$MY_BUNDLE_DIR --qmldir=$$PWD/resources --release --compiler-runtime

    # Copy all resources to build folder
    Resources.path = $$DESTDIR/Resources
    Resources.files += media/*

    # Copy all config files to build folder
    Config.path = $$DESTDIR
    Config.files += config/resources.cfg
    Config.files += config/$$M_BUILD_DIR/plugins.cfg

    Licenses.path = $$DESTDIR/Licenses
    Licenses.files += $$PWD/licenses/*

    INSTALLS += package plugins Resources Config Licenses
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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libsandboxcore/release/ -lsandboxcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libsandboxcore/debug/ -lsandboxcore
else:unix: LIBS += -L$$OUT_PWD/../libsandboxcore/ -lsandboxcore

INCLUDEPATH += $$PWD/../libsandboxcore
DEPENDPATH += $$PWD/../libsandboxcore

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QOgre/release/ -lQOgre
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QOgre/debug/ -lQOgre
else:unix: LIBS += -L$$OUT_PWD/../QOgre/ -lQOgre

INCLUDEPATH += $$PWD/../QOgre
DEPENDPATH += $$PWD/../QOgre
