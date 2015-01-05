#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "PluginManager.h"

#include <QObject>
#include <QTime>
#include <QAtomicInt>

class OgreEngine;
class Scene;
class QTimerEvent;

namespace Ogre
{
class SceneManager;
}

class SceneManager : public QObject
{
    Q_OBJECT
public:
    explicit SceneManager(OgreEngine* engine);

    void pause();
    void start();
    void update(float deltaTime);
    bool isPlaying() const;

    // __speedFactor__ must be in range between __sMinimumSpeedFactor__ and __sMaximumSpeedFactor__
    void setSimulationSpeed(float speedFactor);

    OgreEngine* getOgreEngine();

    Scene* getCurrentScene();
    const Scene* getCurrentScene() const;

    Scene* loadScene(const QString& name, const QString& sceneFile, const QString& logicFile);
    void unloadCurrentScene();
signals:
    void timePassed(const QTime& time);
protected:
    void timerEvent(QTimerEvent *);
private:
    PluginManager mPluginManager;
    OgreEngine* mOgreEngine;
    Scene* mCurrentScene;
    QAtomicInt mSceneStarted;
    float mSimulationSpeedFactor;
    QTime mLastUpdateTime;

    static const float sMinimumSpeedFactor;
    static const float sMaximumSpeedFactor;
    static const float sMaximumTickRate;
    static const float sAITickRate;
};

#endif // SCENEMANAGER_H
