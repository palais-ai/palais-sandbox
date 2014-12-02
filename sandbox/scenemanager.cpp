#include "scenemanager.h"
#include "sceneloader.h"
#include "scene.h"

#include "../libqmlogre/ogreengine.h"

#include <QtGlobal>
#include <QTimerEvent>
#include <QTime>

#include <OgreRoot.h>
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
    assert(thread() == mOgreEngine->thread()); // This object must reside in the same thread as the ogre engine

    startTimer(1000.f / sMaximumTickRate);
}

Scene* SceneManager::loadScene(const QString& name, const QString& sceneFile, const QString& logicFile)
{
    if(mOgreEngine && mSceneManager)
    {
        if(mCurrentScene)
        {
            delete mCurrentScene;
            mCurrentScene = NULL;
        }

        Scene* nextScene = SceneLoader::loadScene(mOgreEngine, mSceneManager, name, sceneFile, logicFile);

        if(!nextScene)
        {
            qWarning("Scene %s could not be loaded.", name.toStdString().c_str());
            return NULL;
        }

        return mCurrentScene = nextScene;
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

bool SceneManager::isPlaying() const
{
    return mSceneStarted;
}

void SceneManager::timerEvent(QTimerEvent*)
{
    static QTime before = QTime::currentTime();

    QTime now = QTime::currentTime();
    float accumulator = before.msecsTo(now) * mSimulationSpeedFactor;

    if(mCurrentScene && mSceneStarted)
    {
        float oneStep =  1000.f / sAITickRate;
        while(accumulator > oneStep)
        {
            // We update at constant rates to keep the results the same, independent of simulation speed.
            mCurrentScene->update(oneStep);
            accumulator -= oneStep;
        }
    }

    // Don't forget the leftovers.
    before = now.addMSecs(-accumulator);
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

const Scene* SceneManager::getCurrentScene() const
{
    return mCurrentScene;
}
