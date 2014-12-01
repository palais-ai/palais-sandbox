#include "scene.h"

#include <OgreSceneNode.h>

Scene::Scene(const QString& sceneFile, const QString& logicFile, Ogre::SceneNode* root) :
    mSceneFile(sceneFile),
    mLogicFile(logicFile),
    mRoot(root)
{
    ;
}


void Scene::update(float time)
{
    QScriptValue fun = mLogicScript.globalObject().property("update");
    fun.call(QScriptValue(), QScriptValueList() << time);
}

void Scene::performAction(const QString& actionName, const QVariant& params)
{
    ;
}

QVariant Scene::getGlobalKnowledge(const QString& knowledgeKey) const
{
    return QVariant();
}

void Scene::setGlobalKnowledge(const QString& knowledgeKey, const QVariant& value)
{
    ;
}

QVariant Scene::getActorKnowledge(const QString& actorName, const QString& knowledgeKey) const
{
    return QVariant();
}

void Scene::setActorKnowledge(const QString& actorName, const QString& knowledgeKey, const QVariant& value)
{
    ;
}

QScriptEngine& Scene::getScriptEngine()
{
    return mLogicScript;
}

