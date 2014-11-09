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

        FrameworkFiles.files = $$OGREDIR/lib/release/Ogre.framework
        FrameworkFiles.path = Contents/Frameworks

        ConfigFiles.files += config/resources.cfg config/plugins.cfg
        ConfigFiles.path = Contents/MacOS

        PluginFiles.files = $$OGREDIR/lib/RenderSystem_GL.dylib
        PluginFiles.path = Contents/Plugins

        MediaFiles.files = media/SdkTrays.zip media/data.zip
        MediaFiles.path = Contents/MacOS/Resources

        QMAKE_BUNDLE_DATA += FrameworkFiles ConfigFiles PluginFiles MediaFiles
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

        CONFIG(release, debug|release) {
            DESTDIR = $$OUT_PWD/release
        } else {
            DESTDIR = $$OUT_PWD/debug
        }

        message(Putting libraries in $$DESTDIR)
        message(QDIRs are in $$[QT_INSTALL_LIBS])

        package.path = $$DESTDIR
        package.files += $$OGREDIR/bin/release/opt/*.dll $$OGREDIR/bin/release/*.dll
        package.files += $$OGREDIR/bin/debug/opt/*.dll $$OGREDIR/bin/debug/*.dll
        package.files += $$[QT_INSTALL_LIBS]/../bin/*.dll
        package.CONFIG = no_check_exist

        # Copy all resources to build folder
        Resources.path = $$DESTDIR/Resources
        Resources.files = media/*.zip

        # Copy all config files to build folder
        Config.path = $$DESTDIR
        Config.files += config/resources.cfg

        CONFIG(release, debug|release) {
            Config.files += config/release/plugins.cfg
        } else {
            Config.files += config/debug/plugins.cfg
        }

        INSTALLS += package Resources Config

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
                LIBS += -L$$BOOSTDIR/lib -llibboost_date_time-vc100-mt-1_49 -llibboost_thread-vc100-mt-1_49
            } else {
                LIBS += -L$$BOOSTDIR/lib -llibboost_date_time-vc100-mt-gd-1_49 -llibboost_thread-vc100-mt-gd-1_49
            }
        }
    }
}

RESOURCES += resources/resources.qrc

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
