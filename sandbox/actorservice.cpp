#include "actorservice.h"
#include "scenemanager.h"
#include "scene.h"

ActorService::ActorService(SceneManager& sceneManager, QObject *parent) :
    QJsonRpcService(parent),
    mSceneManager(sceneManager)
{
    ;
}

const QVariant& ActorService::getKnowledge(const QString& actorName, const QString& knowledgeKey) const
{
    Scene* scene = mSceneManager.getCurrentScene();
    if(!scene)
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::InternalError, "There's no active scene right now.");
        currentRequest().respond(msg);
    }

    if(!scene->hasActorKnowledge(actorName, knowledgeKey))
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::UserError,
                                                                             QString("There's no knowledge for key __%1__.").arg(knowledgeKey));
        currentRequest().respond(msg);
    }

    return scene->getActorKnowledge(actorName, knowledgeKey);
}

void ActorService::setKnowledge(const QString& actorName, const QString& knowledgeKey, const QVariant& value)
{
    Scene* scene = mSceneManager.getCurrentScene();
    if(!scene)
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::InternalError, "There's no active scene right now.");
        currentRequest().respond(msg);
    }

    scene->setActorKnowledge(actorName, knowledgeKey, value);
}

const QVariant& ActorService::performAction(const QString& actorName, const QString& actionName, const QVariant& params)
{
    Scene* scene = mSceneManager.getCurrentScene();
    if(!scene)
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::InternalError, "There's no active scene right now.");
        currentRequest().respond(msg);
    }

    scene->performAction(actionName, params);

    return QVariant();
}
