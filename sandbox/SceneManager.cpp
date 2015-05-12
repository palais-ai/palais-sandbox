#include "scenemanager.h"
#include "sceneloader.h"
#include "Scene.h"
#include "application.h"
#include "utility/timedlogger.h"
#include "Bindings/JavascriptBindings.h"
#include "QOEngine.h"
#include <QtGlobal>
#include <QTimerEvent>
#include <QTime>
#include <OgreRoot.h>
#include <OgreSceneManager.h>

const float SceneManager::sMinimumSpeedFactor = 0.1f; // In times the normal speed
const float SceneManager::sMaximumSpeedFactor = 10.f;  // In times the normal speed
const float SceneManager::sMaximumTickRate    = 100.f; // In ticks Per Second
const float SceneManager::sAITickRate         = 100.f; // In ticks Per Second

SceneManager::SceneManager(QOEngine* engine) :
    mEngine(engine),
    mCurrentScene(NULL),
    mSceneStarted(false),
    mSimulationSpeedFactor(1),
    mAccumulator(0),
    mLastUpdateTime(QTime::currentTime())
{
    assert(engine);
    // This object must reside in the same thread as the ogre engine
    assert(thread() == mEngine->thread());

    mPluginManager.loadPlugins();

    startTimer(1000.f / sMaximumTickRate);
}

void SceneManager::unloadCurrentScene()
{
    if(mCurrentScene)
    {
        mPluginManager.sceneEnded(*mCurrentScene);
        mCurrentScene->teardown();
        delete mCurrentScene;
        mCurrentScene = NULL;
    }
}

Scene* SceneManager::loadScene(const QString& name,
                               const QString& sceneFile,
                               const QString& logicFile)
{
    TimedLogger logger;
    logger.start();

    if(mEngine)
    {       
        unloadCurrentScene();

        Ogre::SceneManager* scnMgr = Ogre::Root::getSingleton()
                                                .getSceneManager(Application::sSceneManagerName);

        Scene* nextScene = SceneLoader::loadScene(mEngine,
                                                  scnMgr,
                                                  name,
                                                  sceneFile,
                                                  logicFile,
                                                  mPluginManager);

        if(!nextScene)
        {
            qWarning("Scene %s could not be loaded.", name.toStdString().c_str());
            return NULL;
        }

        logger.stop("Scene load");
        return mCurrentScene = nextScene;
    }
    else
    {
        qFatal("The ogre engine has to be instantiated to be able to load scenes.");
        return NULL;
    }
}

void SceneManager::pause()
{
    mSceneStarted = false;
}

void SceneManager::start()
{
    mLastUpdateTime = QTime::currentTime();
    mSceneStarted = true;
}

bool SceneManager::isPlaying() const
{
    return mSceneStarted;
}

void SceneManager::timerEvent(QTimerEvent*)
{
    QTime now = QTime::currentTime();

    if(mCurrentScene && mSceneStarted)
    {
        mAccumulator += mLastUpdateTime.msecsTo(now) * mSimulationSpeedFactor;

        const float accumBefore = mAccumulator;

        float oneStep =  1000.f / sAITickRate;
        while(mAccumulator > oneStep)
        {
            // We update at constant rates to keep the results the same,
            // independent of simulation speed.
            mCurrentScene->update(oneStep);
            mPluginManager.update(*mCurrentScene, oneStep);
            mAccumulator -= oneStep;
        }

        QTime passed(0, 0);
        passed = passed.addMSecs(accumBefore - mAccumulator);
        emit timePassed(passed);
    }

    mLastUpdateTime = now;
}

void SceneManager::setSimulationSpeed(float speedFactor)
{
    if(speedFactor >= sMinimumSpeedFactor && speedFactor <= sMaximumSpeedFactor)
    {
        mSimulationSpeedFactor = speedFactor;
    }
    else
    {
        qWarning("Tried to set simulation speed to %.2f, \
                  which is out of the valid range [%.2f, %.2f].",
                 speedFactor, sMinimumSpeedFactor, sMaximumSpeedFactor);
    }
}

QOEngine* SceneManager::getEngine()
{
    return mEngine;
}

Scene* SceneManager::getCurrentScene()
{
    return mCurrentScene;
}

const Scene* SceneManager::getCurrentScene() const
{
    return mCurrentScene;
}