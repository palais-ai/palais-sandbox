#include "scenedynamics.h"
#include "ogrehelper.h"

#include <QScopedArrayPointer>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <OgreSceneNode.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreEntity.h>

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

SceneDynamics::SceneDynamics() :
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
}

SceneDynamics::~SceneDynamics()
{
    ;
}

void SceneDynamics::update(float deltaTime)
{
    Q_UNUSED(deltaTime);
    mWorld->updateAabbs();
    mWorld->computeOverlappingPairs();
}

void SceneDynamics::addPhysicsBody(Ogre::SceneNode* node,
                                   float mass,
                                   float friction,
                                   float restitution,
                                   short filter)
{

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

    btVector3 localInertia(0, 0, 0);
    if(mass != btScalar(0.))
    {
        shape->calculateLocalInertia(mass, localInertia);
    }

    // Optimize the convex shape by using the shape hull utility.
    btShapeHull* hull = new btShapeHull(shape);
    btScalar margin = shape->getMargin();
    hull->buildHull(margin);
    verticesBegin = reinterpret_cast<const btScalar*>(hull->getVertexPointer());
    btConvexHullShape* simplifiedConvexShape = new btConvexHullShape(verticesBegin,
                                                                     hull->numVertices());

    delete hull;
    delete shape;
    shape = simplifiedConvexShape;

    OgreMotionState* motionState = new OgreMotionState(node);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
    rbInfo.m_restitution = restitution;
    rbInfo.m_friction = friction;

    btRigidBody* body = new btRigidBody(rbInfo);
    mWorld->addRigidBody(body, 1, filter);

    btTransform transform;
    motionState->getWorldTransform(transform);
    body->setWorldTransform(transform);
    body->setCollisionShape(shape);
    body->setCollisionFlags(body->getCollisionFlags() |
                            btCollisionObject::CF_KINEMATIC_OBJECT |
                            btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}
