#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "scenemanager.h"
#include <QObject>

class OgreEngine;
class Scene;

namespace Ogre {
class SceneManager;
}

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(OgreEngine* engine, Ogre::SceneManager* sceneManager);

    bool getSceneLoaded() const;
    bool isPlaying() const;
    void play();
    void pause();
signals:
    void startSceneLoad(const QString& sceneFile, const QString& logicFile);
    void sceneLoaded(Scene* scene);
    void sceneLoadFailed(const QString& errorMessage);
public slots:
    void onOpenProject(const QUrl& url);
private:
    SceneManager mScenarioManager;
};

#endif // PROJECTMANAGER_H
