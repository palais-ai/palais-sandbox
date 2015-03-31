#include "Scene.h"
#include "Actor.h"
#include "Bindings/JavascriptBindings.h"
#include "DebugDrawer.h"
#include "../libqmlogre/cameranodeobject.h"
#include "../libqmlogre/ogreengine.h"
#include <cassert>
#include <QDebug>
#include <QVector>
#include <QQuickWindow>
#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager.h>
#include <OgreEntity.h>
#include <OgreAnimationState.h>
#include <OgreRay.h>
#include <OgreSceneQuery.h>
#include <OgreStringConverter.h>

Scene::Scene(const QString& name,
             const QString& sceneFile,
             const QString& logicFile,
             Ogre::SceneNode* root,
             OgreEngine* engine,
             const QString& sceneManagerName) :
    mName(name),
    mSceneFile(sceneFile),
    mLogicFile(logicFile),
    mEngine(engine),
    mSceneManagerName(sceneManagerName),
    mRoot(root),
    mRayQuery(getOgreSceneManager()->createRayQuery(Ogre::Ray())),
    mSphereQuery(getOgreSceneManager()->createSphereQuery(Ogre::Sphere())),
    mIsSetup(false)
{
    if(!mEngine)
    {
        qFatal("Scene requires an engine instance.");
    }

    // This object must reside in the same thread as the ogre engine.
    assert(thread() == mEngine->thread());
    getActors(mRoot);
    Ogre::Root::getSingleton().addFrameListener(this);
}

Scene::~Scene()
{
    Ogre::Root::getSingleton().removeFrameListener(this);

    foreach(DebugDrawer* drawer, mDrawers)
    {
        qWarning() << "User forgot to destroy debug drawer [" << drawer->getName().c_str() << "]."
                   << "Cleaning up automatically.";
        delete drawer;
    }

    if(mRayQuery)
    {
        getOgreSceneManager()->destroyQuery(mRayQuery);
        mRayQuery = NULL;
    }

    for(QMap<QString, Actor*>::iterator it = mActors.begin(); it != mActors.end(); ++it)
    {
        delete it.value();
    }
}

Ogre::SceneManager* Scene::getOgreSceneManager() const
{
    return Ogre::Root::getSingleton()
            .getSceneManager(mSceneManagerName.toStdString());
}

DebugDrawer* Scene::createDebugDrawer(const QString& name)
{
    mDrawers += new DebugDrawer(name.toStdString(), getOgreSceneManager(), 0.5);
    return mDrawers.last();
}

void Scene::destroyDebugDrawer(DebugDrawer* drawer)
{
    int idx = mDrawers.indexOf(drawer);

    if(idx != -1)
    {
        delete mDrawers[idx];
        mDrawers.remove(idx);
    }
    else
    {
        qWarning("Tried to remove inexistant debug drawer.");
    }
}

void Scene::setCameraFocus(Actor* actor)
{
    if(!actor)
    {
        qWarning() << "Actor must be initialized";
        return;
    }

    const QString cameraName("cam1");
    CameraNodeObject* camera = mEngine->getQQuickWindow()
                                      ->findChild<CameraNodeObject*>(cameraName);

    if(!camera)
    {
        qFatal("Couldn't find camera with name (objectName=%s).",
               cameraName.toLocal8Bit().constData());
        return;
    }

    camera->focus(actor->getSceneNode());
}

void Scene::attach(Actor* actor)
{
    actor->getSceneNode()->getParent()->removeChild(actor->getSceneNode());
    getOgreSceneManager()->getRootSceneNode()->addChild(actor->getSceneNode());
}

bool Scene::frameStarted(const Ogre::FrameEvent& evt)
{
    Q_UNUSED(evt);
    foreach(DebugDrawer* drawer, mDrawers)
    {
        drawer->build();
    }
    return true;
}

bool Scene::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    Q_UNUSED(evt);
    return true;
}

bool Scene::frameEnded(const Ogre::FrameEvent& evt)
{
    Q_UNUSED(evt);
    return true;
}

Actor* Scene::getActorForNode(Ogre::SceneNode* node) const
{
    QMap<QString, Actor*>::const_iterator it = mActors.begin();
    for(; it != mActors.end(); ++it)
    {
        if(it.value()->getSceneNode() == node)
        {
            return it.value();
        }
    }
    return NULL;
}

RaycastResult Scene::raycast(const Ogre::Vector3& origin,
                             const Ogre::Vector3& direction)
{
    Ogre::Ray ray(origin, direction.normalisedCopy());

    mRayQuery->setRay(ray);
    mRayQuery->setSortByDistance(true);
    mRayQuery->setQueryTypeMask(Ogre::SceneManager::ENTITY_TYPE_MASK);
    Ogre::RaySceneQueryResult& result = mRayQuery->execute();

    RaycastResult retVal;
    retVal.actor = NULL;
    retVal.distance = 0;

    if(!result.empty())
    {
        Ogre::MovableObject* obj = result.front().movable;
        Ogre::SceneNode* node = obj->getParentSceneNode();

        if(!node)
        {
            qWarning("No parent node attached to movable object %s in raycast query.",
                     obj->getName().c_str());
            return retVal;
        }

        Actor* actor = getActorForNode(node);

        if(!actor)
        {
            qWarning("No actor found for scene node %s in raycast query.",
                     node->getName().c_str());
            return retVal;
        }

        retVal.actor = actor;
        retVal.distance = result.front().distance;
    }

    mRayQuery->clearResults();
    return retVal;
}

RangeQueryResult Scene::rangeQuery(const Ogre::Vector3& origin, float distance)
{
    mSphereQuery->setSphere(Ogre::Sphere(origin, distance));
    mSphereQuery->setQueryTypeMask(Ogre::SceneManager::ENTITY_TYPE_MASK);
    Ogre::SceneQueryResultMovableList& result = mSphereQuery->execute().movables;

    RangeQueryResult retVal;
    for(Ogre::SceneQueryResultMovableList::iterator it = result.begin();
        it != result.end(); ++it)
    {
        Ogre::MovableObject* obj = *it;
        Ogre::SceneNode* node = obj->getParentSceneNode();

        if(!node)
        {
            qWarning("No parent node attached to movable object %s in sphere query.",
                     obj->getName().c_str());
            return retVal;
        }

        Actor* actor = getActorForNode(node);

        if(!actor)
        {
            qWarning("No actor found for scene node %s in sphere query.",
                     node->getName().c_str());
            return retVal;
        }

        retVal.actors += actor;
    }

    mSphereQuery->clearResults();
    return retVal;
}

// CREDITS: http://www.ogre3d.org/forums/viewtopic.php?f=2&t=53647&start=0
void Scene::destroyAllAttachedMovableObjects( Ogre::SceneNode* i_pSceneNode )
{
   if ( !i_pSceneNode )
   {
      assert( false );
      return;
   }

   // Destroy all the attached objects
   Ogre::SceneNode::ObjectIterator itObject = i_pSceneNode->getAttachedObjectIterator();

   while ( itObject.hasMoreElements() )
   {
      Ogre::MovableObject* pObject = static_cast<Ogre::MovableObject*>(itObject.getNext());
      i_pSceneNode->getCreator()->destroyMovableObject( pObject );
   }

   // Recurse to child SceneNodes
   Ogre::SceneNode::ChildNodeIterator itChild = i_pSceneNode->getChildIterator();

   while ( itChild.hasMoreElements() )
   {
      Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
      destroyAllAttachedMovableObjects( pChildNode );
   }
}

void Scene::toggleHighlight(bool highlighted, int index)
{
    if(index >= 0 && index < mActors.size())
    {
        mActors.values().at(index)->toggleHighlight(highlighted);
    }
    else
    {
        qWarning("Tried to access actor index beyond bounds idx=%d.", index);
    }
}

const QString& Scene::getName() const
{
    return mName;
}

const QMap<QString, Actor*>& Scene::getActors() const
{
    return mActors;
}

Actor* Scene::instantiate(const QString& name,
                          const QString& meshName,
                          const Ogre::Vector3& position,
                          const Ogre::Quaternion& rotation,
                          const Ogre::Vector3& scale)
{
    if(mActors.contains(name))
    {
        qWarning() << "Tried to instantiate an actor with name "
                   << name
                   << " that already existed in the scene. The instantiation is not performed.";
        return NULL;
    }

    Ogre::SceneManager* scnMgr = getOgreSceneManager();

    Ogre::String meshFile = meshName.toStdString() + ".mesh";
    qDebug() << "Loading mesh at " << QString::fromStdString(meshFile);

    try
    {
        Ogre::MeshManager::getSingleton().load(meshFile, "General");
    }
    catch(Ogre::Exception &/*e*/)
    {
        qWarning() << "Failed to load mesh at " << QString::fromStdString(meshFile)
                   << ". Instantiation of actor " << name << " failed.";
        return NULL;
    }

    Ogre::SceneNode* node = scnMgr->getRootSceneNode()->createChildSceneNode(name.toStdString(),
                                                                             position,
                                                                             rotation);
    node->setScale(scale);

    Ogre::Entity* entity = scnMgr->createEntity(name.toStdString(), meshFile);
    entity->setCastShadows(true);
    node->attachObject(entity);

    Actor* retVal = addActor(node);

    return retVal;
}

void Scene::destroy(Actor* actor)
{
    if(!actor)
    {
        qWarning() << "Tried to destroy a null actor. The destruction can't be performed.";
        return;
    }

    if(!mActors.contains(actor->getName()))
    {
        qWarning() << "Tried to destroy an actor with name " << actor->getName()
                   << " that didnt exist in the scene. The destruction is not performed.";
        return;
    }

    destroyAllAttachedMovableObjects(actor->getSceneNode());
    actor->getSceneNode()->getCreator()->destroySceneNode(actor->getSceneNode());

    JavaScriptBindings::removeActorBinding(actor, mLogicScript);

    int numRemoved = mActors.remove(actor->getName());
    qDebug() << "[" << actor->getName() << "] : Num. Removed: " << numRemoved;
    assert(numRemoved == 1);
    emit actorRemoved(actor->getName());
    emit actorRemovedObject(actor);
    emit actor->removedFromScene(actor);
    delete actor;
}

Actor* Scene::getActor(unsigned int index)
{
    return mActors.values()[index];
}

Actor* Scene::getActor(const QString& actorName)
{
    if(mActors.contains(actorName))
    {
        return mActors[actorName];
    }
    else
    {
        return NULL;
    }
}

void Scene::onActorChangeVisible(const QString& actorName, bool visible)
{
    Actor* actor = getActor(actorName);

    if(!actor)
    {
        qWarning("Can't change visiblity of actor %s, because it isn't in the scene.",
                 actorName.toLocal8Bit().constData());
        return;
    }

    actor->setVisible(visible);
}

void Scene::onActorVisibilityChanged(Actor* actor, bool visible)
{
    emit actorChangedVisibility(actor->getName(), visible);
}

QObjectList Scene::getActorsArray() const
{
    QObjectList list;

    for(QMap<QString, Actor*>::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
    {
        list += it.value();
    }

    return list;
}

void Scene::onRequestEmitCurrentActors()
{
    foreach(Actor* actor, mActors)
    {
        emit actorAdded(actor->getName());
    }
}

Actor* Scene::addActor(Ogre::SceneNode* node)
{
    QString name = node->getName().c_str();

    Actor* newActor = new Actor(node);
    mActors[name] = newActor;
    connect(newActor, &Actor::visibilityChanged,
            this, &Scene::onActorVisibilityChanged);

    // FIXME: Integrate physics properly.
    //mDynamics.addPhysicsBody(newActor->getSceneNode());
    JavaScriptBindings::addActorBinding(newActor, mLogicScript);

    emit actorAdded(name);

    return mActors[name];
}

void Scene::getActors(Ogre::SceneNode* root)
{
    if(!root)
    {
        return;
    }

    Ogre::SceneNode::ChildNodeIterator children = root->getChildIterator();
    while (children.hasMoreElements())
    {
        Ogre::SceneNode* child = static_cast<Ogre::SceneNode*>(children.getNext());
        // Recursively add children
        getActors(child);
        addActor(child);
    }
}

void Scene::setup()
{
    QScriptValue fun = mLogicScript.globalObject().property("onStart");

    if(fun.isFunction())
    {
        fun.call();

        JavaScriptBindings::checkScriptEngineException(mLogicScript, "onStart");
    }
    else
    {
        qWarning("No __onStart__ handler defined in script.");
    }
}

void Scene::update(float time)
{
    if(!mIsSetup)
    {
        setup();
        mIsSetup = true;
    }

    mDynamics.update(time);

    float deltaTimeInSeconds = time / 1000.f;

    // Call into the scripts __update__ function.
    QScriptValue fun = mLogicScript.globalObject().property("update");
    if(fun.isFunction())
    {
        fun.call(QScriptValue(), QScriptValueList() << deltaTimeInSeconds);

        JavaScriptBindings::checkScriptEngineException(mLogicScript, "update");
    }
    else
    {
        qWarning("No __update__ handler defined in script.");
    }

    // Update timers.
    JavaScriptBindings::timers_update(deltaTimeInSeconds);

    // Update all actors.
    for(QMap<QString, Actor*>::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
    {
        it.value()->update(deltaTimeInSeconds);
    }
}

void Scene::performAction(const QString& actionName,
                          const QVariant& params)
{
    QScriptValue fun = mLogicScript.globalObject().property(actionName);
    if(fun.isFunction())
    {
        QScriptValueList list;
        if(params.isValid())
        {
                list << mLogicScript.newVariant(params);
        }

        fun.call(QScriptValue(), list);

        JavaScriptBindings::checkScriptEngineException(mLogicScript,
                                                       "performAction( " + actionName + " )");
    }
}

QScriptEngine& Scene::getScriptEngine()
{
    return mLogicScript;
}

const QString& Scene::getLogicFile() const
{
    return mLogicFile;
}

const QString& Scene::getSceneFile() const
{
    return mSceneFile;
}

