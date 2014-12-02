#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QString>
#include <QObject>

class OgreEngine;
class Scene;
class QTimerEvent;

namespace Ogre {
class SceneManager;
}

class SceneManager : public QObject
{
    Q_OBJECT
public:
    SceneManager(OgreEngine* engine, Ogre::SceneManager* sceneManager);

    void pause();
    void start();
    void update(float deltaTime);
    bool isPlaying() const;

    // __speedFactor__ must be in range between __sMinimumSpeedFactor__ and __sMaximumSpeedFactor__
    void setSimulationSpeed(float speedFactor);

    Scene* getCurrentScene();
    const Scene* getCurrentScene() const;
public slots:
    Scene* loadScene(const QString& name, const QString& sceneFile, const QString& logicFile);
protected:
    void timerEvent(QTimerEvent *);
private:
    OgreEngine* mOgreEngine;
    Ogre::SceneManager* mSceneManager;
    Scene* mCurrentScene;
    bool mSceneStarted;
    float mSimulationSpeedFactor;

    static const float sMinimumSpeedFactor = 0.1f; // In times the normal speed
    static const float sMaximumSpeedFactor = 5.f;  // In times the normal speed
    static const float sMaximumTickRate = 100.f; // In Ticks Per Second
    static const float sAITickRate = 100.f; // In Ticks Per Second
};

#endif // SCENEMANAGER_H
