#include "sceneloader.h"
#include "scene.h"

#include <QtGlobal>

#include <OgreSceneManager.h>
#include <OgreString.h>

#include "../libqmlogre/ogreengine.h"
#include "../libdotsceneloader/DotSceneLoader.h"

SceneLoader::SceneLoader()
{
}

Scene* SceneLoader::loadScene(OgreEngine* engine, Ogre::SceneManager* sceneManager, const QString& sceneFile, const QString& logicFile)
{
    if(engine && sceneManager)
    {
        loadSceneVisuals(engine, sceneManager, sceneFile);
        return 0;
    }
    else
    {
        qFatal("An initialized ogre engine and scene manager are required to load scenes.");
        return 0;
    }
}

void SceneLoader::loadSceneVisuals(OgreEngine* engine, Ogre::SceneManager* sceneManager, const QString& sceneFile)
{
    if(engine && sceneManager)
    {
        engine->lockEngine();
        // Resources with textures must be loaded within Ogre's GL context
        engine->activateOgreContext();

        Ogre::DotSceneLoader dsloader;
        dsloader.parseDotScene(Ogre::String(sceneFile.toUtf8()), "General", sceneManager);

        engine->doneOgreContext();
        engine->unlockEngine();
    }
    else
    {
        qFatal("An initialized ogre engine and scene manager are required to load scenes.");
    }
}

void SceneLoader::loadSceneLogic(const QString& logicFile)
{

}
