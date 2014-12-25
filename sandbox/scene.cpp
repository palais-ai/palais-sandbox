#include "scene.h"
#include "actor.h"
#include "application.h"
#include "javascriptbindings.h"
#include "ogrehelper.h"
#include "timedlogger.h"
#include "astar.h"

#include <cassert>

#include <QDebug>

#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <OgreMeshManager.h>
#include <OgreEntity.h>
#include <OgreAnimationState.h>
#include <OgreRay.h>
#include <OgreSceneQuery.h>
#include <OgreStringConverter.h>

#include "../libqmlogre/ogreengine.h"

Scene::Scene(const QString& name, const QString& sceneFile, const QString& logicFile, Ogre::SceneNode* root, OgreEngine* engine) :
    mName(name),
    mSceneFile(sceneFile),
    mLogicFile(logicFile),
    mRoot(root),
    mEngine(engine),
    mRayQuery(Ogre::Root::getSingleton().getSceneManager(Application::sSceneManagerName)->createRayQuery(Ogre::Ray())),
    mDebugDrawer(Ogre::Root::getSingleton().getSceneManager(Application::sSceneManagerName), 0.5f),
    mIsSetup(false)
{
    if(!mEngine)
    {
        qFatal("Scene requires an engine instance.");
    }

    getActors(mRoot);

    Ogre::Root::getSingleton().addFrameListener(this);
}

Scene::~Scene()
{
    Ogre::Root::getSingleton().removeFrameListener(this);

    if(mRayQuery)
    {
        Ogre::Root::getSingleton().getSceneManager(Application::sSceneManagerName)->destroyQuery(mRayQuery);
        mRayQuery = NULL;
    }

    for(QMap<QString, Actor*>::iterator it = mActors.begin(); it != mActors.end(); ++it)
    {
        delete it.value();
    }
}


bool Scene::frameStarted(const Ogre::FrameEvent& evt)
{
    mDebugDrawer.build();
}

bool Scene::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    ;
}

bool Scene::frameEnded(const Ogre::FrameEvent& evt)
{
    ;
}

Actor* Scene::getActorForNode(Ogre::SceneNode* node) const
{
    for(QMap<QString, Actor*>::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
    {
        if(it.value()->getSceneNode() == node)
        {
            return it.value();
        }
    }
    return NULL;
}

RaycastResult Scene::raycast(const Ogre::Vector3& origin, const Ogre::Vector3& direction)
{
    mEngine->lockEngine();

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
            qWarning("No parent node attached to movable object %s in raycast query.", obj->getName().c_str());
            mEngine->unlockEngine();
            return retVal;
        }

        Actor* actor = getActorForNode(node);

        if(!actor)
        {
            qWarning("No actor found for scene node %s in raycast query.", node->getName().c_str());
            mEngine->unlockEngine();
            return retVal;
        }

        retVal.actor = actor;
        retVal.distance = result.front().distance;
    }

    mRayQuery->clearResults();
    mEngine->unlockEngine();
    return retVal;
}

static ailib::real_type euclideanHeuristic(const OgreHelper::NavigationGraph::node_type& n1,
                                           const OgreHelper::NavigationGraph::node_type& n2)
{
    return n1.getCentroid().distance(n2.getCentroid());
}

Q_DECLARE_METATYPE(Ogre::Vector3)
Q_DECLARE_METATYPE(Ogre::Vector3*)

void Scene::moveActor(Actor* actor, const Ogre::Vector3& target)
{
    const OgreHelper::TriangleNode* start = OgreHelper::getNavNodeClosestToPoint(mNavMesh, actor->getPosition());
    if(!start)
    {
        qWarning() << "Actor [ " << actor->getName() << " ]'s start position is not covered by the navmesh. No valid path could be calculated.";
        return;
    }

    const OgreHelper::TriangleNode* goal = OgreHelper::getNavNodeClosestToPoint(mNavMesh, target);
    if(!goal)
    {
        qWarning() << "Target position " << Ogre::StringConverter::toString(target).c_str() << " is not covered by the navmesh. No valid path could be calculated.";
        return;
    }

    ailib::AStar<OgreHelper::NavigationGraph> astar;
    ailib::AStar<OgreHelper::NavigationGraph>::path_type path = astar.find_path(mNavMesh, start, goal, euclideanHeuristic);

    qDebug() << "path size is " << path.size() << "hops.";

    if(path.size() == 1 && path[0] == start)
    {
        qDebug() << "Target has already been reached.";
        actor->setKnowledge("movement_target", QVariant::fromValue(start->getCentroid()));
        return;
    }

    QVector<Ogre::Vector3> qpath;

    for(ailib::AStar<OgreHelper::NavigationGraph>::path_type::const_iterator it = path.begin(); it != path.end(); ++it)
    {
        qpath += (*it)->getCentroid();
    }
    actor->setKnowledge("current_path", QVariant::fromValue(qpath));
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

int Scene::size() const
{
    return mActors.size();
}

const QString& Scene::getName() const
{
    return mName;
}

QHash<int, QByteArray> Scene::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ModelRoleName] = "name";
    roles[ModelRoleIndex] = "index";
    return roles;
}

Qt::ItemFlags Scene::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant Scene::data(const QModelIndex &index, int role) const
{
    Actor* actor = mActors.values().at(index.row());

    switch(role)
    {
    case ModelRoleName:
        return actor->getName();
    case ModelRoleIndex:
        return index.row();
    default:
        return QVariant(QVariant::Invalid);
    }
}

QVariant Scene::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant(QVariant::Invalid);
}

int Scene::rowCount(const QModelIndex &parent) const
{
    return mActors.size();
}

const QMap<QString, Actor*>& Scene::getActors() const
{
    return mActors;
}

Actor* Scene::instantiate(const QString& name, const QString& meshName, const Ogre::Vector3& position, const Ogre::Quaternion& rotation, const Ogre::Vector3& scale)
{
    if(mActors.contains(name))
    {
        qWarning() << "Tried to instantiate an actor with name " << name << " that already existed in the scene. The instantiation is not performed.";
        return NULL;
    }

    mEngine->lockEngine();

    Ogre::SceneManager* scnMgr = Ogre::Root::getSingleton().getSceneManager(Application::sSceneManagerName);

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

    Ogre::SceneNode* node = scnMgr->getRootSceneNode()->createChildSceneNode(name.toStdString(), position, rotation);
    node->setScale(scale);

    Ogre::Entity* entity = scnMgr->createEntity(name.toStdString(), meshFile);
    entity->setCastShadows(true);
    node->attachObject(entity);

    mEngine->unlockEngine();

    Actor* retVal = new Actor(node);

    QMap<QString, Actor*> actorsCopy = mActors;
    actorsCopy[name] = retVal;
    const int index = actorsCopy.values().indexOf(retVal);

    beginInsertRows( QModelIndex(), index, index);
    mActors[name] = retVal;
    endInsertRows();

    JavaScriptBindings::addActorBinding(retVal, mLogicScript);

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

    mEngine->lockEngine();

    Ogre::SceneManager* scnMgr = Ogre::Root::getSingleton().getSceneManager(Application::sSceneManagerName);

    destroyAllAttachedMovableObjects(actor->getSceneNode());
    scnMgr->destroySceneNode(actor->getSceneNode());

    mEngine->unlockEngine();

    JavaScriptBindings::removeActorBinding(actor, mLogicScript);

    const int index = mActors.values().indexOf(actor);

    beginRemoveRows( QModelIndex(), index, index);
    mActors.remove(actor->getName());
    endRemoveRows();
}

Actor* Scene::getActor(unsigned int index)
{
    return mActors.values()[index];
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

QVariantMap& Scene::getKnowledge()
{
    return mKnowledge;
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

        QString name = child->getName().c_str();

        mActors[name] = new Actor(child);

        if(name.toLower() == "navmesh")
        {
            parseNavMesh(mActors[name]);
        }
    }
}

void Scene::parseNavMesh(Actor* navmesh)
{
    TimedLogger logger;
    logger.start();
    mNavMesh = OgreHelper::makeNavGraphFromOgreNode(navmesh->getSceneNode());
    logger.stop("NavMesh parsing");

    for(OgreHelper::NavigationGraph::node_collection::const_iterator it = mNavMesh.nodes.begin(); it != mNavMesh.nodes.end(); ++it)
    {
        mDebugDrawer.drawCircle(it->getCentroid(), 0.1, 4, Ogre::ColourValue::Green, true);

        qDebug() << "edgecount: " << it->edges.size();
        for(ailib::BaseNode::edge_collection::const_iterator it2 = it->edges.begin(); it2 != it->edges.end(); ++it2)
        {
            mDebugDrawer.drawLine(it->getCentroid(),
                                  static_cast<const OgreHelper::NavigationGraph::node_type*>(it2->target)->getCentroid(),
                                  Ogre::ColourValue::Red);
        }
    }

    qDebug() << "Navigation mesh loaded with " << mNavMesh.nodes.size() << " nodes.";
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
        qWarning("No onStart handler defined in script.");
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
    QScriptValue fun = mLogicScript.globalObject().property("update");
    if(fun.isFunction())
    {
        fun.call(QScriptValue(), QScriptValueList() << deltaTimeInSeconds);

        JavaScriptBindings::checkScriptEngineException(mLogicScript, "update");
    }
    else
    {
        qWarning("No update handler defined in script.");
    }

    JavaScriptBindings::timers_update(deltaTimeInSeconds);

    for(QMap<QString, Actor*>::const_iterator it = mActors.begin(); it != mActors.end(); ++it)
    {
        it.value()->update(deltaTimeInSeconds);
    }
}

void Scene::performAction(const QString& actionName, const QVariant& params)
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

        JavaScriptBindings::checkScriptEngineException(mLogicScript, "performAction( " + actionName + " )");
    }
}

bool Scene::hasKnowledge(const QString& knowledgeKey) const
{
    return mKnowledge.contains(knowledgeKey);
}

QVariant Scene::getKnowledge(const QString& knowledgeKey) const
{
    return mKnowledge[knowledgeKey];
}

void Scene::setKnowledge(const QString& knowledgeKey, const QVariant& value)
{
    mKnowledge[knowledgeKey] = value;
}

bool Scene::hasActorKnowledge(const QString& actorName, const QString& knowledgeKey) const
{
    return mActors[actorName]->hasKnowledge(knowledgeKey);
}

QVariant Scene::getActorKnowledge(const QString& actorName, const QString& knowledgeKey) const
{
    return mActors[actorName]->getKnowledge(knowledgeKey);
}

void Scene::setActorKnowledge(const QString& actorName, const QString& knowledgeKey, const QVariant& value)
{
    mActors[actorName]->setKnowledge(knowledgeKey, value);
}

QScriptEngine& Scene::getScriptEngine()
{
    return mLogicScript;
}

