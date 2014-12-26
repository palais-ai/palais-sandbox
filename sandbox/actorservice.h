#ifndef ACTORSERVICE_H
#define ACTORSERVICE_H

#undef QJSONRPC_BUILD
#include "../libqjsonrpc/src/qjsonrpcservice.h"

class SceneManager;

class ActorService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", "ActorService")
public:
    explicit ActorService(SceneManager& sceneManager, QObject *parent = 0);

signals:

public slots:
    QVariant getKnowledge(const QString& actorName,
                          const QString& knowledgeKey);
    void setKnowledge(const QString& actorName,
                      const QString& knowledgeKey,
                      const QVariant& value);
    QVariant performAction(const QString& actorName,
                           const QString& actionName,
                           const QVariant& params = QVariant());
private:
    SceneManager& mSceneManager;
};

#endif // ACTORSERVICE_H
