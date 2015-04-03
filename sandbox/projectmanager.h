#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "scenemanager.h"
#include <QObject>
#include <QUrl>

class Actor;
class OgreEngine;
class Scene;
class CameraNodeObject;
class KnowledgeModel;

namespace Ogre
{
class SceneManager;
}

class ProjectManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
public:
    explicit ProjectManager(OgreEngine* engine);
    ~ProjectManager();

    bool getSceneLoaded() const;
    bool isPlaying() const;
    void play();
    void pause();

    // Not thread-safe.
    // Do not call this from any other thread than this QObject's thread().
    void initializeSceneManager();

    // Thread-safe
    Q_INVOKABLE void setSimulationSpeed(float speedFactor);

    // Thread-safe
    Q_INVOKABLE void reloadProject();

    // Thread-safe
    Q_INVOKABLE void unselectActor();
signals:
    void startSceneLoad(const QString& sceneFile,
                        const QString& logicFile);
    void sceneLoaded(Scene* scene);
    void sceneLoadFailed(const QString& errorMessage);
    void beforeSceneLoad(const QString& name,
                         const QString& sceneFile,
                         const QString& logicFile);
    void playingChanged(bool isPlaying);
    void inspectorSelectionChanged(QString name,
                                   QVariantMap initial);
    void inspectorResetModel(const KnowledgeModel* knowledge);
    void timePassed(const QTime& time);
    void actorChangedSelected(const QString& actorName,
                              bool selected);
    void signalSetSimulationSpeed(float speedFactor);
    void signalReloadProject();
    void signalUnselectActor();
    void oneSecondTimeout();
public slots:
    void onSetSimulationSpeed(float speedFactor);
    void onReloadProject();
    void onActorChangeSelected(const QString& actorName,
                               bool selected);
    void onTimePassed(const QTime& time);
    void onOpenProject(const QUrl url);
    void onSceneSetupFinished();
    // Coordinates are in normalized screen coordinates.
    void onSelectActorAtClickpoint(float mouseX,
                                   float mouseY);
    void onFocusSelectedActor();
    void onSaveRenderView(const QUrl& url);
    void onUnselectActor();
    void onActorRemoved(const QString& actorName);
protected:
    void timerEvent(QTimerEvent *);
private:
    static std::string sCurrentResourceGroupName;
    void loadResources(const QStringList& paths);
    void prepareScene(CameraNodeObject* camera);
    CameraNodeObject* getCameraWithName(const QString& name);

    QUrl mLastOpenedUrl, mCurrentProjectUrl;
    SceneManager mScenarioManager;
    Actor* mSelectedActor;
};

#endif // PROJECTMANAGER_H
