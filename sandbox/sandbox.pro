CONFIG += qt
QT += qml quick
TEMPLATE = app
TARGET = sandbox

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

SOURCES += main.cpp \
    cameranodeobject.cpp \
    exampleapp.cpp

HEADERS += cameranodeobject.h \
    exampleapp.h

OTHER_FILES += \
    resources/example.qml

macx {
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error(QmlOgre needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        message(Using Ogre libraries in $$OGREDIR)
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
        QMAKE_LFLAGS += -F$$OGREDIR/lib/release
        LIBS += -framework Ogre -framework OpenGL

        BOOSTDIR = $$OGREDIR/boost
        !isEmpty(BOOSTDIR) {
            INCLUDEPATH += $$BOOSTDIR
#            LIBS += -L$$BOOSTDIR/lib -lboost_date_time-xgcc40-mt-1_42 -lboost_thread-xgcc40-mt-1_42
        }
    }
} else:unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE
} else:win32 {
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error(QmlOgre needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        message(Using Ogre libraries in $$OGREDIR)
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
        CONFIG(release, debug|release) {
            LIBS += -L$$OGREDIR/lib/release -L$$OGREDIR/lib/release/opt -lOgreMain -lRenderSystem_GL
        } else {
            LIBS += -L$$OGREDIR/lib/debug -L$$OGREDIR/lib/debug/opt -lOgreMain_d -lRenderSystem_GL_d
        }

        BOOSTDIR = $$OGREDIR/boost
        !isEmpty(BOOSTDIR) {
            INCLUDEPATH += $$BOOSTDIR
            CONFIG(release, debug|release) {
                LIBS += -L$$BOOSTDIR/lib -llibboost_date_time-vc90-mt-1_42 -llibboost_thread-vc90-mt-1_42
            } else {
                LIBS += -L$$BOOSTDIR/lib -llibboost_date_time-vc90-mt-gd-1_42 -llibboost_thread-vc90-mt-gd-1_42
            }
        }
    }
}

RESOURCES += resources/resources.qrc

# Copy all resources to build folder
Resources.path = $$OUT_PWD/resources
Resources.files = resources/*.zip

# Copy all config files to build folder
Config.path = $$OUT_PWD
Config.files = config/*

# make install
INSTALLS += Resources Config

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libqmlogre/release/ -lqmlogre
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libqmlogre/debug/ -lqmlogre
else:unix: LIBS += -L$$OUT_PWD/../libqmlogre/ -lqmlogre

INCLUDEPATH += $$PWD/../libqmlogre
DEPENDPATH += $$PWD/../libqmlogre

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libqmlogre/release/libqmlogre.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libqmlogre/debug/libqmlogre.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libqmlogre/release/qmlogre.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../libqmlogre/debug/qmlogre.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../libqmlogre/libqmlogre.a

macx {
    FrameworkFiles.files = $$OGREDIR/lib/release/Ogre.framework
    FrameworkFiles.path = Contents/Frameworks

    ConfigFiles.files = config/plugins.cfg config/resources.cfg
    ConfigFiles.path = Contents/MacOS

    PluginFiles.files = $$OGREDIR/lib/RenderSystem_GL.dylib
    PluginFiles.path = Contents/Plugins

    MediaFiles.files = media/SdkTrays.zip media/data.zip
    MediaFiles.path = Contents/MacOS/Resources

    QMAKE_BUNDLE_DATA += FrameworkFiles ConfigFiles PluginFiles MediaFiles
}
