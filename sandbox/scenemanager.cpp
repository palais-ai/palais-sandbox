#include "scenemanager.h"
#include "sceneloader.h"
#include "scene.h"

#include <QTimerEvent>
#include <QTime>

SceneManager::SceneManager() :
    mCurrentScene(NULL),
    mSceneStarted(false),
    mSimulationSpeedFactor(1)
{
    startTimer(100.f / sMaximumTickRate);
}

Scene* SceneManager::loadScene(const QString& sceneFile, const QString& logicFile)
{
    return SceneLoader::loadScene(sceneFile, logicFile);
}

void SceneManager::pause()
{
    mSceneStarted = false;
}

void SceneManager::start()
{
    mSceneStarted = true;
}

void SceneManager::timerEvent(QTimerEvent*)
{
    static QTime before = QTime::currentTime();

    QTime now = QTime::currentTime();
    if(mCurrentScene && mSceneStarted)
    {
        mCurrentScene->update(before.msecsTo(now) * mSimulationSpeedFactor);
    }

    before = now;
}

void SceneManager::setSimulationSpeed(float speedFactor)
{
    if(speedFactor > sMinimumSpeedFactor && speedFactor < sMaximumSpeedFactor)
    {
        mSimulationSpeedFactor = speedFactor;
    }
}

Scene* SceneManager::getCurrentScene()
{
    return 0;
}
