#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

class Scene;
class ProjectManager;

namespace Ogre {
class SceneManager;
class Root;
}

class OgreEngine;
class QQmlApplicationEngine;
class CameraNodeObject;

class Application : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool sceneLoaded READ getSceneLoaded NOTIFY onSceneLoadedChanged)
    Q_PROPERTY(bool scenePlaying READ getScenePlaying NOTIFY onScenePlayingChanged)
public:
    explicit Application(QObject *parent = 0);
    ~Application();

    int onApplicationStarted(int argc, char **argv);
    qreal loadingProgress() const;
    bool getSceneLoaded() const;
    bool getScenePlaying() const;
signals:
    void ogreInitialized();
    void beforeSceneLoadFinished(const QString& name, const QString& sceneFile, const QString& logicFile);
    void onSceneLoadedChanged(bool sceneLoaded);
    void onScenePlayingChanged(bool scenePlaying);
public slots:
    void initializeOgre();
    void onOgreIsReady();
    void onSceneLoaded(Scene* scene);
    void onSceneLoadFailed(const QString& message);
    void onPlayButtonPressed();
    void onBeforeSceneLoad(const QString& name, const QString& sceneFile, const QString& logicFile);
private:
    CameraNodeObject* getCameraWithName(const QString& name);

    QQmlApplicationEngine* mApplicationEngine;
    OgreEngine *mOgreEngine;
    Ogre::SceneManager *mSceneManager;
    ProjectManager* mProjectManager;
    Ogre::Root *mRoot;
};

#endif // APPLICATION_H
