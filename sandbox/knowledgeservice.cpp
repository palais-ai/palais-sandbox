#include "knowledgeservice.h"
#include "scenemanager.h"
#include "scene.h"

#include <QDebug>

KnowledgeService::KnowledgeService(SceneManager& sceneManager, QObject *parent) :
    QJsonRpcService(parent),
    mSceneManager(sceneManager)
{
    ;
}

QVariant KnowledgeService::getKnowledge(const QString& knowledgeKey)
{
    Scene* scene = mSceneManager.getCurrentScene();
    if(!scene)
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::UserError, "There's no active scene right now.");
        beginDelayedResponse();
        currentRequest().respond(msg);
        return QVariant(QVariant::Invalid);
    }

    if(!scene->hasKnowledge(knowledgeKey))
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::UserError,
                                                                             QString("There's no knowledge for key __%1__.").arg(knowledgeKey));
        beginDelayedResponse();
        currentRequest().respond(msg);
        return QVariant(QVariant::Invalid);
    }

    return scene->getKnowledge(knowledgeKey);
}

void KnowledgeService::setKnowledge(const QString& knowledgeKey, const QVariant& value)
{
    Scene* scene = mSceneManager.getCurrentScene();
    if(!scene)
    {
        QJsonRpcMessage msg = currentRequest().request().createErrorResponse(QJsonRpc::UserError, "There's no active scene right now.");
        beginDelayedResponse();
        currentRequest().respond(msg);
        return;
    }

    scene->setKnowledge(knowledgeKey, value);
}

