#ifndef KNOWLEDGESERVICE_H
#define KNOWLEDGESERVICE_H

#include "../libqjsonrpc/src/qjsonrpcservice.h"

class SceneManager;

class KnowledgeService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", "KnowledgeService")
public:
    explicit KnowledgeService(SceneManager& sceneManager, QObject *parent = 0);

public slots:
    const QVariant& getKnowledge(const QString& knowledgeKey) const;
    void setKnowledge(const QString& knowledgeKey, const QVariant& value);
private:
    SceneManager& mSceneManager;
};

#endif // KNOWLEDGESERVICE_H
