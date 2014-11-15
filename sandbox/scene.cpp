#include "scene.h"

Scene::Scene(const QString& sceneFile, const QString& logicFile) :
    mSceneFile(sceneFile),
    mLogicFile(logicFile)
{
    ;
}


void Scene::update(float time)
{
    ;
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

