BULLETDIR = $$(BULLET_HOME)
isEmpty(BULLETDIR) {
    error($$TARGET needs bullet to be built. Please set the environment variable BULLET_HOME pointing to your bullet root directory.)
} else {
    message(Using bullet libraries in $$BULLETDIR)

    INCLUDEPATH += $$BULLETDIR/src

    macx {
        BULLET_BUILD_DIR = $$BULLETDIR/src
    } else:win32 {
        BULLET_BUILD_DIR = $$BULLETDIR/lib
    }
    LIBS += -L$$BULLET_BUILD_DIR -L$$BULLET_BUILD_DIR/BulletCollision -L$$BULLET_BUILD_DIR/BulletDynamics -L$$BULLET_BUILD_DIR/LinearMath -lBulletCollision -lBulletDynamics -lLinearMath
}
