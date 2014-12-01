#include "scenemanager.h"
#include "sceneloader.h"
#include "scene.h"

#include <QtGlobal>
#include <QTimerEvent>
#include <QTime>

#include <OgreSceneManager.h>

SceneManager::SceneManager(OgreEngine* engine, Ogre::SceneManager* sceneManager) :
    mOgreEngine(engine),
    mSceneManager(sceneManager),
    mCurrentScene(NULL),
    mSceneStarted(false),
    mSimulationSpeedFactor(1)
{
    assert(engine);
    assert(sceneManager);

    startTimer(1000.f / sMaximumTickRate);
}

Scene* SceneManager::loadScene(const QString& sceneFile, const QString& logicFile)
{
    if(mCurrentScene)
    {
        delete mCurrentScene;
        mCurrentScene = NULL;
    }

    if(mOgreEngine && mSceneManager)
    {
        return mCurrentScene = SceneLoader::loadScene(mOgreEngine, mSceneManager, sceneFile, logicFile);
    }
    else
    {
        qFatal("The ogre engine or the scene manager have to be instantiated to be able to load scenes.");
        return NULL;
    }
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
    else
    {
        qWarning("Tried to set simulation speed to %.2f, which is out of the valid range [%.2f, %.2f].",
                 speedFactor, sMinimumSpeedFactor, sMaximumSpeedFactor);
    }
}

Scene* SceneManager::getCurrentScene()
{
    return mCurrentScene;
}
