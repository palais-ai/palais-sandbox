#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QString>
#include <QObject>
#include <QTime>

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
    explicit SceneManager(OgreEngine* engine);

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
    Scene* mCurrentScene;
    bool mSceneStarted;
    float mSimulationSpeedFactor;
    QTime mLastUpdateTime;

    static const float sMinimumSpeedFactor;
    static const float sMaximumSpeedFactor;
    static const float sMaximumTickRate;
    static const float sAITickRate;
};

#endif // SCENEMANAGER_H
