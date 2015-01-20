#ifndef SCENEDYNAMICS_H
#define SCENEDYNAMICS_H

#include <QScopedPointer>

class btSequentialImpulseConstraintSolver;
struct btDbvtBroadphase;
class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;

namespace Ogre
{
class SceneNode;
}

class SceneDynamics
{
public:
    SceneDynamics();
    ~SceneDynamics();

    void addPhysicsBody(Ogre::SceneNode* node,
                        float mass = 1.f,
                        float friction = 0.75f,
                        float restitution = 0.5f,
                        short filter = 1);

    void update(float deltaTime);
private:
    QScopedPointer<btDefaultCollisionConfiguration> mCollisionConfiguration;
    QScopedPointer<btCollisionDispatcher> mCollisionDispatcher;
    QScopedPointer<btDbvtBroadphase> mBroadphase;
    QScopedPointer<btSequentialImpulseConstraintSolver> mSolver;
    QScopedPointer<btDiscreteDynamicsWorld> mWorld;
};

#endif // SCENEDYNAMICS_H
