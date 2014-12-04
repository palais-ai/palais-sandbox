#include "scene.h"
#include "actor.h"

#include <cassert>

#include <QtDebug>

#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

#include "../libqmlogre/ogreengine.h"

Scene::Scene(const QString& name, const QString& sceneFile, const QString& logicFile, Ogre::SceneNode* root, OgreEngine* engine) :
    mName(name),
    mSceneFile(sceneFile),
    mLogicFile(logicFile),
    mRoot(root),
    mEngine(engine)
{
    if(!mEngine)
    {
        qFatal("Scene requires an engine instance.");
    }

    getActors(mRoot);
}

Scene::~Scene()
{
    for(QMap<QString, Actor*>::iterator it = mActors.begin(); it != mActors.end(); ++it)
    {
        delete it.value();
    }
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

Actor* Scene::instantiate(const QString& name, Actor* prototype, const QVector3D& position, const QQuaternion& rotation)
{
    return NULL;
}

void Scene::destroy(Actor* actor)
{

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
    }
}

void Scene::setup()
{
    QScriptValue fun = mLogicScript.globalObject().property("onStart");

    if(fun.isFunction())
    {
        fun.call();

        checkScriptEngineException("onStart");
    }
}

void Scene::checkScriptEngineException(const QString& context)
{
    if(mLogicScript.hasUncaughtException())
    {
        if(context.isEmpty())
        {
            qWarning() << "Exception in loaded logic file "
                       << mLogicFile << ", ERROR:" << mLogicScript.uncaughtException().toString();
        }
        else
        {
            qWarning() << "Exception in " << context << ", ERROR:" << mLogicScript.uncaughtException().toString();
        }
    }
}

void Scene::update(float time)
{
    QScriptValue fun = mLogicScript.globalObject().property("update");
    if(fun.isFunction())
    {
        fun.call(QScriptValue(), QScriptValueList() << time / 1000.f);

        checkScriptEngineException("update");
    }
}

void Scene::performAction(const QString& actionName, const QVariant& params)
{
    QScriptValue fun = mLogicScript.globalObject().property(actionName);
    if(fun.isFunction())
    {
        fun.call(QScriptValue(), QScriptValueList() << mLogicScript.newVariant(params));

        checkScriptEngineException("performAction( " + actionName + " )");
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

