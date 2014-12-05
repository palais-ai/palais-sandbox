#include "knowledgeservice.h"
#include "scenemanager.h"
#include "scene.h"

KnowledgeService::KnowledgeService(SceneManager& sceneManager, QObject *parent) :
    QJsonRpcService(parent),
    mSceneManager(sceneManager)
{
    ;
}

const QVariant& KnowledgeService::getKnowledge(const QString& knowledgeKey) const
{
    Scene* scene = mSceneManager.getCurrentScene();
    if(!scene)
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::InternalError, "There's no active scene right now.");
        currentRequest().respond(msg);
    }

    if(!scene->hasKnowledge(knowledgeKey))
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::UserError,
                                                                             QString("There's no knowledge for key __%1__.").arg(knowledgeKey));
        currentRequest().respond(msg);
    }

    return scene->getKnowledge(knowledgeKey);
}

void KnowledgeService::setKnowledge(const QString& knowledgeKey, const QVariant& value)
{
    Scene* scene = mSceneManager.getCurrentScene();
    if(!scene)
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::InternalError, "There's no active scene right now.");
        currentRequest().respond(msg);
    }

    scene->setKnowledge(knowledgeKey, value);
}

