OGREDIR = $$(OGRE_HOME)
isEmpty(OGREDIR) {
    error($$TARGET needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
} else {
    message(Using Ogre libraries in $$OGREDIR)
}

macx {
    INCLUDEPATH += $$OGREDIR/include/OGRE
    INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
    LIBS += -framework Ogre -framework OpenGL

    QMAKE_LFLAGS += -F$$OGREDIR/lib/release

    BOOSTDIR = $$OGREDIR/boost
    !isEmpty(BOOSTDIR) {
        INCLUDEPATH += $$BOOSTDIR
        LIBS += -L$$BOOSTDIR/lib -lboost_date_time-xgcc42-mt-1_49 -lboost_thread-xgcc42-mt-1_49
    }
} else:win32 {
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
