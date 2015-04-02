

macx {
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error($$TARGET needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        message(Using Ogre libraries in $$OGREDIR)
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
        LIBS += -framework Ogre -framework OpenGL

        !exists($$OGREDIR/lib/macosx) { # Ogre 1.8
message(ogre 1.8)
            QMAKE_LFLAGS += -F$$OGREDIR/lib/release
        } else { # Ogre 1.9
message(ogre 1.9)
            QMAKE_LFLAGS += -F$$OGREDIR/lib/macosx/Release
        }

        BOOSTDIR = $$OGREDIR/boost
        !isEmpty(BOOSTDIR) {
            INCLUDEPATH += $$BOOSTDIR
            !exists($$OGREDIR/lib/macosx) { # Ogre 1.8
                LIBS += -L$$BOOSTDIR/lib -lboost_date_time-xgcc42-mt-1_49 -lboost_thread-xgcc42-mt-1_49
            } else { # Ogre 1.9
                LIBS += -L$$BOOSTDIR/lib -lboost_date_time -lboost_chrono -lboost_system -lboost_thread
            }
        }

    }
} else:unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE
} else:win32 {
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error($$TARGET needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
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
                LIBS += -L$$BOOSTDIR/lib -llibboost_date_time-vc100-mt-1_49 -llibboost_thread-vc100-mt-1_49
            } else {
                LIBS += -L$$BOOSTDIR/lib -llibboost_date_time-vc100-mt-gd-1_49 -llibboost_thread-vc100-mt-gd-1_49
            }
        }
    }
}
