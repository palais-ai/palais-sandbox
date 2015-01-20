BULLETDIR = $$(BULLET_HOME)
isEmpty(BULLETDIR) {
    error($$TARGET needs bullet to be built. Please set the environment variable BULLET_HOME pointing to your bullet root directory.)
} else {
    message(Using bullet libraries in $$BULLETDIR)

    INCLUDEPATH += $$BULLETDIR/src
    LIBS += -L$$BULLETDIR/src/BulletCollision -L$$BULLETDIR/src/BulletDynamics -L$$BULLETDIR/src/LinearMath -lBulletCollision -lBulletDynamics -lLinearMath
}
