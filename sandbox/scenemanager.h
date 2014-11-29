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

    Scene* loadScene(const QString& sceneFile, const QString& logicFile);

    void pause();
    void start();
    void update(float deltaTime);

    // __speedFactor__ must be in range between __sMinimumSpeedFactor__ and __sMaximumSpeedFactor__
    void setSimulationSpeed(float speedFactor);

    Scene* getCurrentScene();
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
    static const float sMaximumTickRate = 200.f; // In Ticks Per Second
};

#endif // SCENEMANAGER_H
