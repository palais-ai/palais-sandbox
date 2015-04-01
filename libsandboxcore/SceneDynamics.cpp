#include "SceneDynamics.h"
#include "Actor.h"
#include "Scene.h"
#include "OgreHelper.h"
#include "DebugDrawer.h"
#include <QScopedArrayPointer>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btIDebugDraw.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <OgreSceneNode.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreEntity.h>
#include <qDebug>

// CREDITS: From http://www.bulletphysics.org/mediawiki-1.5.8/index.php/MotionStates
class OgreMotionState : public btMotionState
{
protected:
    Ogre::SceneNode* mSceneNode;

    btTransform transformFromNode() const
    {
        const Ogre::Vector3& position = mSceneNode->getPosition();
        const Ogre::Quaternion& orientation = mSceneNode->getOrientation();

        btMatrix3x3 basis;
        basis.setIdentity();

        btTransform transform;
        transform.setIdentity();
        transform.setBasis(basis);
        transform.setOrigin(btVector3(position.x, position.y, position.z));
        transform.setRotation(btQuaternion(orientation.x,
                                           orientation.y,
                                           orientation.z,
                                           orientation.w));
        return transform;
    }
public:
    explicit OgreMotionState(Ogre::SceneNode *node = NULL)
    {
        mSceneNode = node;
    }

    virtual ~OgreMotionState()
    {
    }

    void setNode(Ogre::SceneNode *node)
    {
        mSceneNode = node;
    }

    virtual void getWorldTransform(btTransform &worldTrans) const
    {
        worldTrans = transformFromNode();
    }

    virtual void setWorldTransform(const btTransform &worldTrans)
    {
        if(mSceneNode == NULL)
        {
            return; // silently return before we set a node
        }

        btQuaternion rot = worldTrans.getRotation();
        mSceneNode->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
        btVector3 pos = worldTrans.getOrigin();
        mSceneNode->setPosition(pos.x(), pos.y(), pos.z());
    }
};

class BulletDebugDrawer : public btIDebugDraw
{
    DebugDrawer* mDrawer;
    int mDebugMode;

public:

    DebugDrawer* getDebugDrawer()
    {
        return mDrawer;
    }

    BulletDebugDrawer(DebugDrawer* drawer) :
        mDrawer(drawer)
    {

    }

    virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
    {
        mDrawer->drawLine(Ogre::Vector3(from.getX(), from.getY(), from.getZ()),
                          Ogre::Vector3(to.getX(), to.getY(), to.getZ()),
                          Ogre::ColourValue(color.getX(), color.getY(), color.getZ()));
    }

    virtual void drawContactPoint(const btVector3& PointOnB,
                                  const btVector3& normalOnB,
                                  btScalar distance,
                                  int lifeTime,
                                  const btVector3& color)
    {
        // NOP
    }

    virtual void reportErrorWarning(const char* warningString)
    {
        qWarning() << warningString;
    }

    virtual void draw3dText(const btVector3& location,const char* textString)
    {
        // NOP
    }

    virtual void setDebugMode(int debugMode)
    {
        mDebugMode = debugMode;
    }

    virtual int getDebugMode() const
    {
        return mDebugMode;
    }
};

SceneDynamics::SceneDynamics(Scene& scene) :
    mScene(scene),
    mCollisionConfiguration(new btDefaultCollisionConfiguration),
    mCollisionDispatcher(new btCollisionDispatcher(mCollisionConfiguration.data())),
    mBroadphase(new btDbvtBroadphase),
    mSolver(new btSequentialImpulseConstraintSolver),
    mWorld(new btDiscreteDynamicsWorld(mCollisionDispatcher.data(),
                                       mBroadphase.data(),
                                       mSolver.data(),
                                       mCollisionConfiguration.data()))
{
    // CREDITS: From CCDPhysicsDemo.cpp (Bullet SDK Demo)
    mWorld->getSolverInfo().m_solverMode |= SOLVER_USE_2_FRICTION_DIRECTIONS|SOLVER_RANDMIZE_ORDER;
    mWorld->setGravity(btVector3(0, -9.81, 0));
    mWorld->setDebugDrawer(new BulletDebugDrawer(mScene.createDebugDrawer("BulletDrawer")));
}

SceneDynamics::~SceneDynamics()
{
    BulletDebugDrawer* bDrawer = static_cast<BulletDebugDrawer*>(mWorld->getDebugDrawer());
    mScene.destroyDebugDrawer(bDrawer->getDebugDrawer());
}

void SceneDynamics::addPhysicsBody(Actor* actor,
                                   float mass,
                                   float friction,
                                   float restitution,
                                   short filter)
{
    if(actor->getName().startsWith("Cube"))
    {
        return;
    }

    Ogre::SceneNode* node = actor->getSceneNode();
    if(node->numAttachedObjects() == 0)
    {
        return;
    }

    Ogre::Entity* entity = dynamic_cast<Ogre::Entity*>(node->getAttachedObject(0));

    if(!entity)
    {
        return;
    }

    const Ogre::Vector3& position = node->getPosition();
    const Ogre::Vector3& scale = node->getScale();
    const Ogre::Quaternion& orientation = node->getOrientation();
    const Ogre::MeshPtr mesh = entity->getMesh();

    if(mesh.isNull())
    {
        return;
    }

    size_t vertexCount, indexCount;
    Ogre::Vector3* verticesPtr;
    unsigned* indicesPtr;
    OgreHelper::getMeshInformation(mesh,
                                   vertexCount,
                                   verticesPtr,
                                   indexCount,
                                   indicesPtr,
                                   position,
                                   orientation,
                                   scale);

    QScopedArrayPointer<Ogre::Vector3> vertices(verticesPtr);
    QScopedArrayPointer<unsigned> indices(indicesPtr);

    const btScalar* verticesBegin = reinterpret_cast<const btScalar*>(vertices.data());
    btConvexHullShape* shape = new btConvexHullShape(verticesBegin,
                                                     vertexCount);

    shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));
    btVector3 localInertia(0, 0, 0);
    if(mass != btScalar(0.))
    {
        shape->calculateLocalInertia(mass, localInertia);
    }

    // Optimize the convex shape by using the shape hull utility.
    // CREDITS: http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=BtShapeHull_vertex_reduction_utility
    btShapeHull* hull = new btShapeHull(shape);
    btScalar margin = shape->getMargin();
    hull->buildHull(margin);
    verticesBegin = reinterpret_cast<const btScalar*>(hull->getVertexPointer());
    btConvexHullShape* simplifiedConvexShape = new btConvexHullShape(verticesBegin,
                                                                     hull->numVertices());

    qWarning() << "Num. Vertices in body : " << vertexCount << " reduced to " << simplifiedConvexShape->getNumVertices();

    delete hull;
    //delete shape;
    //shape = simplifiedConvexShape;

    OgreMotionState* motionState = new OgreMotionState(node);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
    rbInfo.m_restitution = restitution;
    rbInfo.m_friction = friction;

    btRigidBody* body = new btRigidBody(rbInfo);
    body->setUserPointer(actor);
    mBodies.insert(actor, QSharedPointer<btRigidBody>(body));
    mWorld->addRigidBody(body, 1, filter);

    btTransform transform;
    motionState->getWorldTransform(transform);
    body->setWorldTransform(transform);
    body->setCollisionShape(shape);
    body->setCollisionFlags(body->getCollisionFlags() |
                            btCollisionObject::CF_KINEMATIC_OBJECT |
                            btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}

void SceneDynamics::removePhysicsBody(Actor* actor)
{
    QHash<Actor*, QSharedPointer<btRigidBody> >::iterator it = mBodies.find(actor);
    if(it == mBodies.end())
    {
        qWarning() << "SceneDynamics.removePhysicsBody: "
                   << "Tried to remove physics body that wasn't added previously.";
        return;
    }

    mWorld->removeRigidBody(it.value().data());
    int numRemoved = mBodies.remove(actor);
    assert(numRemoved == 1);
}

void SceneDynamics::raytest(const Ogre::Vector3& origin,
                            const Ogre::Vector3& direction,
                            Actor** /*out*/ collidedWith,
                            float*  /*out*/ collisionDistance)
{
    assert(collidedWith);
    assert(collisionDistance);

    const static float maxRayRange = 9999;

    Ogre::Vector3 endOgre = origin + maxRayRange*direction;
    btVector3 start = btVector3(origin.x, origin.y, origin.z);
    btVector3 end   = btVector3(endOgre.x, endOgre.y, endOgre.z);

    btCollisionWorld::ClosestRayResultCallback cb(start, end);
    mWorld->rayTest(start, end, cb);

    void* userPtr = NULL;
    if(cb.hasHit() && (userPtr = cb.m_collisionObject->getUserPointer()))
    {
        Actor* actor = static_cast<Actor*>(userPtr);
        *collidedWith = actor;
        *collisionDistance = cb.m_rayFromWorld.distance(cb.m_hitPointWorld);
    }
    else
    {
        *collidedWith = NULL;
        *collisionDistance = 0;
    }
}

class AllContactsResult : public btCollisionWorld::ContactResultCallback
{
public:
    QList<Actor*> contacts;
private:
    btScalar addSingleResult(btManifoldPoint& cp,
                             const btCollisionObjectWrapper* obj,
                             int partId,
                             int index,
                             const btCollisionObjectWrapper* otherObj,
                             int otherPartId,
                             int otherIndex)
     {
        void* userPtr = otherObj->getCollisionObject()->getUserPointer();

        if(!userPtr)
        {
            userPtr = obj->getCollisionObject()->getUserPointer();
        }

        assert(userPtr);
        contacts.append(static_cast<Actor*>(userPtr));
        return true;
     }
};

QList<Actor*> SceneDynamics::rangeTest(const Ogre::Vector3& origin,
                                       float range)
{
    return QList<Actor*>();
    btSphereShape sphere(range);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(1, NULL, &sphere, btVector3(0, 0, 0));
    QScopedPointer<btRigidBody> body(new btRigidBody(rbInfo));
    body->setUserPointer(NULL);
    mWorld->addRigidBody(body.data(), 1, 1);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(origin.x, origin.y, origin.z));
    body->setWorldTransform(transform);
    body->setCollisionShape(&sphere);

    AllContactsResult cb;
    mWorld->contactTest(body.data(), cb);
    mWorld->removeRigidBody(body.data());

    return cb.contacts;
}

void SceneDynamics::update(float deltaTime)
{
    Q_UNUSED(deltaTime);
    BulletDebugDrawer* bDrawer = static_cast<BulletDebugDrawer*>(mWorld->getDebugDrawer());
    bDrawer->getDebugDrawer()->clear();
    mWorld->updateAabbs();
    mWorld->computeOverlappingPairs();
    //mWorld->stepSimulation(deltaTime);
    mWorld->debugDrawWorld();
}
