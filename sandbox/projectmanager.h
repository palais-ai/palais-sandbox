#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "scenemanager.h"
#include "actorservice.h"
#include "knowledgeservice.h"

#include <QObject>
#include <QUrl>

#include "../libqjsonrpc/src/qjsonrpclocalserver.h"

class OgreEngine;
class Scene;

namespace Ogre {
class SceneManager;
class Camera;
}

class ProjectManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool playing READ isPlaying NOTIFY onPlayingChanged)
public:
    explicit ProjectManager(OgreEngine* engine);
    ~ProjectManager();

    // Coordinates are in normalized screen coordinates.
    void selectActorAtClickpoint(float mouseX,
                                 float mouseY,
                                 Ogre::Camera* camera);
    bool getSceneLoaded() const;
    bool isPlaying() const;
    void play();
    void pause();
    Q_INVOKABLE void setSimulationSpeed(float speedFactor);
    Q_INVOKABLE void reloadProject();
signals:
    void startSceneLoad(const QString& sceneFile,
                        const QString& logicFile);
    void sceneLoaded(Scene* scene);
    void sceneLoadFailed(const QString& errorMessage);
    void beforeSceneLoad(const QString& name,
                         const QString& sceneFile,
                         const QString& logicFile);
    void onPlayingChanged(bool isPlaying);
public slots:
    void onOpenProject(const QUrl& url);
    void onBeforeSceneLoadFinished(const QString& name,
                                   const QString& sceneFile,
                                   const QString& logicFile);
private:
    QUrl mLastOpenedUrl, mCurrentProjectUrl;
    SceneManager mScenarioManager;
    KnowledgeService mKnowledgeService;
    ActorService mActorService;
    QJsonRpcLocalServer mServer;
};

#endif // PROJECTMANAGER_H
