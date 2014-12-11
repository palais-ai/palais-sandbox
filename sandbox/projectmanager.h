#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "scenemanager.h"
#include "actorservice.h"
#include "knowledgeservice.h"

#include <QObject>

#include "../libqjsonrpc/src/qjsonrpclocalserver.h"

class OgreEngine;
class Scene;

namespace Ogre {
class SceneManager;
}

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(OgreEngine* engine);
    ~ProjectManager();

    bool getSceneLoaded() const;
    bool isPlaying() const;
    void play();
    void pause();
    Q_INVOKABLE void setSimulationSpeed(float speedFactor);
signals:
    void startSceneLoad(const QString& sceneFile, const QString& logicFile);
    void sceneLoaded(Scene* scene);
    void sceneLoadFailed(const QString& errorMessage);
    void beforeSceneLoad(const QString& name, const QString& sceneFile, const QString& logicFile);
public slots:
    void onOpenProject(const QUrl& url);
    void onBeforeSceneLoadFinished(const QString& name, const QString& sceneFile, const QString& logicFile);
private:
    SceneManager mScenarioManager;
    KnowledgeService mKnowledgeService;
    ActorService mActorService;
    QJsonRpcLocalServer mServer;
};

#endif // PROJECTMANAGER_H
