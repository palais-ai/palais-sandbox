#include "Scene.h"
#include "Actor.h"
#include "Bindings/JavascriptBindings.h"
#include "DebugDrawer.h"
#include "QOCamera.h"
#include "QOEngine.h"
#include <cassert>
#include <algorithm>
#include <QDebug>
#include <QVector>
#include <QQuickWindow>
#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager.h>
#include <OgreEntity.h>
#include <OgreAnimationState.h>
#include <OgreSceneQuery.h>
#include <OgreStringConverter.h>

#define VERBOSE_LOGGING false
#define IF_VERBOSE(x__) do \
{\
    if(VERBOSE_LOGGING) {\
    (x__);}\
} while(false);

Scene::Scene(const QString& name,
             const QString& sceneFile,
             const QString& logicFile,
             Ogre::SceneNode* root,
             QOEngine* engine,
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

    // Connect destroyLater to run from queue.
    connect(this, &Scene::actorDestroyLater,
            this, &Scene::onDestroyLater,
            Qt::QueuedConnection);
}

Scene::~Scene()
{
    Ogre::Root::getSingleton().removeFrameListener(this);

    foreach(DebugDrawer* drawer, mDrawers)
    {
        delete drawer;
    }

    if(mRayQuery)
    {
        getOgreSceneManager()->destroyQuery(mRayQuery);
        mRayQuery = NULL;
    }

    if(mSphereQuery)
    {
        getOgreSceneManager()->destroyQuery(mRayQuery);
        mSphereQuery = NULL;
    }
}

Ogre::SceneManager* Scene::getOgreSceneManager() const
{
    return Ogre::Root::getSingleton().getSceneManager(mSceneManagerName.toStdString());
}

DebugDrawer* Scene::createDebugDrawer(const QString& name, float opacity)
{
    if(mDrawers.contains(name))
    {
        qWarning() << "Scene.createDebugDrawer: Tried to add DebugDrawer with name ["
                   << name << "] that already existed. "
                   << "Names must be unique. Returning NULL.";
        return NULL;
    }
    DebugDrawer* newDrawer = new DebugDrawer(name.toStdString(),
                                             getOgreSceneManager(),
                                             std::min(std::max(opacity, 0.f), 1.f));
    mDrawers[name] = newDrawer;

    // Add the DebugDrawer's scene node to the actor list.
    addActor(getOgreSceneManager()->getSceneNode(name.toStdString()));

    return newDrawer;
}

void Scene::destroyDebugDrawer(DebugDrawer* drawer)
{
    if(!drawer)
    {
        qWarning() << "Scene.destroyDebugDrawer: Can't destroy a NULL drawer.";
        return;
    }

    QString name = QString::fromStdString(drawer->getName());
    QHash<QString, DebugDrawer*>::iterator it = mDrawers.find(name);
    if(it != mDrawers.end())
    {
        DebugDrawer* drawer = it.value();
        mDrawers.remove(name);
        delete drawer;
    }
    else
    {
        qWarning() << "Scene.destroyDebugDrawer: Tried to destroy an inexistant DebugDrawer [ "
                   << name << " ].";
    }
}

DebugDrawer* Scene::createDrawer(const QString& name, float opacity)
{
    return createDebugDrawer(name, opacity);
}

void Scene::destroyDrawer(DebugDrawer* drawer)
{
    destroyDebugDrawer(drawer);
}

void Scene::destroyDrawer(const QString& name)
{
    QHash<QString, DebugDrawer*>::iterator it = mDrawers.find(name);
    if(it != mDrawers.end())
    {
        destroyDebugDrawer(it.value());
    }
    else
    {
        qWarning() << "There's no drawer with the name [" << name << "].";
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
    QOCamera* camera = mEngine->getQQuickWindow()
                                      ->findChild<QOCamera*>(cameraName);

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

QWeakPointer<Actor> Scene::getActorForNode(Ogre::SceneNode* node) const
{
    assert(node);
    QString name = QString::fromStdString(node->getName());
    QHash<QString, QSharedPointer<Actor> >::const_iterator it = mActors.find(name);

    if(it == mActors.end())
    {
        return QWeakPointer<Actor>();
    }
    else
    {
        return QWeakPointer<Actor>(it.value());
    }
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

        QWeakPointer<Actor> actor = getActorForNode(node);
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

        QWeakPointer<Actor> actor = getActorForNode(node);
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
void Scene::destroyAllAttachedMovableObjects(Ogre::SceneNode* i_pSceneNode)
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

      Ogre::Any any = pChildNode->getUserAny();
      if(!any.isEmpty())
      {
          Actor* actor = Ogre::any_cast<Actor*>(any);
          if(actor)
          {
              qDebug() << "Detaching actor before destruction.";
              // Make sure we don't delete any attached actors.
              // Re-attach any found actors to the scene.
              Ogre::Vector3 pos = pChildNode->_getDerivedPosition();
              Ogre::Quaternion rot = pChildNode->_getDerivedOrientation();
              Ogre::Vector3 scale = pChildNode->_getDerivedScale();
              attach(actor);
              actor->setPosition(pos);
              actor->setScale(scale);
              actor->setRotation(rot);
              continue;
          }
      }

      destroyAllAttachedMovableObjects( pChildNode );
   }
}

void Scene::toggleHighlight(const QString& name, bool highlighted)
{
    QHash<QString, QSharedPointer<Actor> >::iterator it = mActors.find(name);
    if(it != mActors.end())
    {
        it.value()->toggleHighlight(highlighted);
    }
    else
    {
        qWarning() << "Scene.toggleHighlight: Tried to access actor that doesn't exist [ "
                   << name << " ].";
    }
}

const QString& Scene::getName() const
{
    return mName;
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
    IF_VERBOSE(qDebug() << "Loading mesh at " << QString::fromStdString(meshFile));

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

    return addActor(node);
}

void Scene::destroy(Actor* actor)
{
    if(!actor)
    {
        qWarning() << "Tried to destroy a null actor. The destruction can't be performed.";
        return;
    }

    QHash<QString, QSharedPointer<Actor> >::iterator it = mActors.find(actor->getName());
    if(it == mActors.end())
    {
        qWarning() << "Tried to destroy an actor with name " << actor->getName()
                   << " that didnt exist in the scene. The destruction is not performed.";
        return;
    }

    // Grab a shared instance so __actor__ doesnt get destroyed before we're done cleaning up.
    QSharedPointer<Actor> actorRef = it.value();

    emit actorRemoved(actorRef->getName());
    emit actorRemovedObject(actorRef.data());
    actorRef->emitSignalBeforeRemoval();

    const int numRemoved = mActors.remove(actorRef->getName());
    IF_VERBOSE(qDebug() << "[" << actorRef->getName() << "] : Num. Removed: " << numRemoved);
    assert(numRemoved == 1);

    JavaScriptBindings::removeActorBinding(actorRef.data(), mLogicScript);

    // Destroy the actor's scene node last. Since it contains data used by the Actor* instance.
    destroyAllAttachedMovableObjects(actorRef->getSceneNode());
    actorRef->getSceneNode()->getCreator()->destroySceneNode(actorRef->getSceneNode());
}

void Scene::destroyLater(Actor* actor)
{
    emit actorDestroyLater(actor);
}

bool Scene::hasActor(const QString& actorName) const
{
    return mActors.contains(actorName);
}

Actor* Scene::getActorByName(const QString& actorName)
{
    return getActor(actorName);
}

Actor* Scene::getActor(const QString& actorName)
{
    QHash<QString, QSharedPointer<Actor> >::iterator it = mActors.find(actorName);
    if(it != mActors.end())
    {
        return it.value().data();
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

void Scene::onDestroyLater(Actor* actor)
{
    destroy(actor);
}

QObjectList Scene::getActorsArray() const
{
    QObjectList list;
    for(QHash<QString, QSharedPointer<Actor> >::const_iterator it = mActors.begin();
        it != mActors.end(); ++it)
    {
        list += it.value().data();
    }
    return list;
}

void Scene::onRequestEmitCurrentActors()
{
    foreach(QSharedPointer<Actor> actor, mActors)
    {
        emit actorAdded(actor->getName());
    }
}

Actor* Scene::addActor(Ogre::SceneNode* node)
{
    QString name = node->getName().c_str();

    Actor* newActor = new Actor(node);

    mActors[name] = QSharedPointer<Actor>(newActor);
    connect(newActor, &Actor::visibilityChanged,
            this, &Scene::onActorVisibilityChanged);

    JavaScriptBindings::addActorBinding(newActor, mLogicScript);

    emit actorAdded(name);
    return mActors[name].data();
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
        if(child->getName() == "Camera")
        {
            continue;
        }

        // Recursively add children
        getActors(child);
        addActor(child);
    }
}

void Scene::setup()
{
    QScriptValue fun = mLogicScript.globalObject().property("onSetup");
    if(fun.isFunction())
    {
        fun.call();
        JavaScriptBindings::checkScriptEngineException(mLogicScript, "onSetup");
    }
    else
    {
        // No __onStart__ handler defined in script. This is not an error. (optional)
    }
}

void Scene::teardown()
{
    QScriptValue fun = mLogicScript.globalObject().property("onTeardown");
    if(fun.isFunction())
    {
        fun.call();
        JavaScriptBindings::checkScriptEngineException(mLogicScript, "onTeardown");
    }
    else
    {
        // No __onTeardown__ handler defined in script. This is not an error. (optional)
    }

    // Delete actors here to trigger any dangling __delete__ events that the script might be listening to.
    // No scripts should be called in the destructor.
    QStringList keys = mActors.keys();
    foreach(const QString& actorName, keys)
    {
        destroy(getActorByName(actorName));
    }
}

void Scene::update(float time)
{
    if(!mIsSetup)
    {
        setup();
        mIsSetup = true;
    }

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
        // No __update__ handler defined in script. This is not an error. (optional)
    }

    // Update timers.
    JavaScriptBindings::timers_update(deltaTimeInSeconds);

    // Update all actors.
    for(QHash<QString, QSharedPointer<Actor> >::const_iterator it = mActors.begin();
        it != mActors.end(); ++it)
    {
        it.value()->update(deltaTimeInSeconds);
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

