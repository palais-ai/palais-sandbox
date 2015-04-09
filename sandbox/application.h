#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QVariant>

class Scene;
class ProjectManager;
class SceneModel;
class InspectorModel;
class ConsoleModel;
class KnowledgeModel;
class TimedLogger;
class LogHandler;

namespace Ogre
{
class SceneManager;
class Root;
class Camera;
}

class QOEngine;
class QQmlApplicationEngine;
class QOCamera;

class Application : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool sceneLoaded READ getSceneLoaded NOTIFY sceneLoadedChanged)
    Q_PROPERTY(bool scenePlaying READ getScenePlaying NOTIFY scenePlayingChanged)
public:
    static const std::string sSceneManagerName;

    explicit Application(QObject *parent = 0);
    ~Application();

    int onApplicationStarted(int argc, char **argv);
    qreal loadingProgress() const;
    bool getSceneLoaded() const;
    bool getScenePlaying() const;

    // Coordinates are in normalized screen coordinates.
    Q_INVOKABLE void onOgreViewClicked(float mouseX, float mouseY);
signals:
    void ogreInitialized();
    void beforeSceneLoadFinished(const QString& name,
                                 const QString& sceneFile,
                                 const QString& logicFile);
    void sceneLoadedChanged(bool sceneLoaded);
    void scenePlayingChanged(bool scenePlaying);
    void sceneSetupFinished();
    void selectActorAtClickpoint(float mouseX,
                                 float mouseY);
    void focusOnSelectedActor();
public slots:
    void initializeOgre();
    void onOgreIsReady();
    void onSceneLoaded(Scene* scene);
    void onCreateProjectFailed(const QString& message);
    void onSceneLoadFailed(const QString& message);
    void onZoomIn();
    void onZoomOut();
    void onPlayButtonPressed();
    void onPlayingChanged(bool isPlaying);
    // Thread-safe
    void onConnectKnowledgeModel(const KnowledgeModel* model);
    void onInspectorSelectionChanged(QString name,
                                     QVariantMap data);
private:
    QQmlApplicationEngine* mApplicationEngine;
    QOEngine* mQOEngine;
    ProjectManager* mProjectManager;
    Ogre::Root* mRoot;
    QScopedPointer<SceneModel> mSceneModel;
    QScopedPointer<InspectorModel> mInspectorModel;
    QScopedPointer<ConsoleModel> mConsoleModel;
    QScopedPointer<TimedLogger> mTimeLogger;
    QSharedPointer<LogHandler> mLogHandler;
};

#endif // APPLICATION_H
