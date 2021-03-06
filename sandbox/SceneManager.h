#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include "PluginManager.h"
#include <QObject>
#include <QTime>
#include <QAtomicInt>

class QOEngine;
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
    explicit SceneManager(QOEngine* engine);

    void update(float deltaTime);
    bool isPlaying() const;

    // __speedFactor__ must be in range between __sMinimumSpeedFactor__ and __sMaximumSpeedFactor__
    void setSimulationSpeed(float speedFactor);

    QOEngine* getEngine();
    Scene* getCurrentScene();
    const Scene* getCurrentScene() const;
    Scene* loadScene(const QString& name, const QString& sceneFile, const QString& logicFile);
    void unloadCurrentScene();
signals:
    void timePassed(const QTime& time);
public slots:
    void pause();
    void start();
protected:
    void timerEvent(QTimerEvent *);
private:
    PluginManager mPluginManager;
    QOEngine* mEngine;
    Scene* mCurrentScene;
    QAtomicInt mSceneStarted;
    float mSimulationSpeedFactor;
    float mAccumulator;
    QTime mLastUpdateTime;

    static const float sMinimumSpeedFactor;
    static const float sMaximumSpeedFactor;
    static const float sMaximumTickRate;
    static const float sAITickRate;
};

#endif // SCENEMANAGER_H
