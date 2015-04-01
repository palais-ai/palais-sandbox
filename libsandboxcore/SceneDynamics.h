#ifndef SCENEDYNAMICS_H
#define SCENEDYNAMICS_H

#include <QScopedPointer>
#include <QSharedPointer>
#include <QHash>

class Actor;
class Scene;
class btRigidBody;
class btSequentialImpulseConstraintSolver;
struct btDbvtBroadphase;
class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;

namespace Ogre
{
class SceneNode;
class Vector3;
}

class SceneDynamics
{
public:
    SceneDynamics(Scene& scene);
    ~SceneDynamics();

    void addPhysicsBody(Actor* actor,
                        float mass = 1.f,
                        float friction = 0.75f,
                        float restitution = 0.5f,
                        short filter = 1);

    void removePhysicsBody(Actor* actor);
    void raytest(const Ogre::Vector3& origin,
                 const Ogre::Vector3& direction,
                 Actor** /*out*/ collidedWith,
                 float*  /*out*/ collisionDistance);
    QList<Actor*> rangeTest(const Ogre::Vector3& origin,
                            float range);
    void update(float deltaTime);
private:
    Scene& mScene;
    QScopedPointer<btDefaultCollisionConfiguration> mCollisionConfiguration;
    QScopedPointer<btCollisionDispatcher> mCollisionDispatcher;
    QScopedPointer<btDbvtBroadphase> mBroadphase;
    QScopedPointer<btSequentialImpulseConstraintSolver> mSolver;
    QScopedPointer<btDiscreteDynamicsWorld> mWorld;
    QHash<Actor*, QSharedPointer<btRigidBody> > mBodies;
};

#endif // SCENEDYNAMICS_H
