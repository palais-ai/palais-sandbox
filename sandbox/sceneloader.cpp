#include "sceneloader.h"
#include "scene.h"
#include "actor.h"
#include "javascriptbindings.h"
#include "application.h"

#include <exception>

#include <QScopedPointer>
#include <QtGlobal>

#include <OgreSceneManager.h>
#include <OgreString.h>
#include <OgreSceneNode.h>

#include "../libqmlogre/ogreengine.h"
#include "../libdotsceneloader/DotSceneLoader.h"

SceneLoader::SceneLoader()
{
}

Scene* SceneLoader::loadScene(OgreEngine* engine,
                              Ogre::SceneManager* sceneManager,
                              const QString& name,
                              const QString& sceneFile,
                              const QString& logicFile)
{
    if(engine && sceneManager)
    {
        loadSceneVisuals(engine, sceneManager, sceneFile);

        QString sceneMgrName = QString("%1").arg(Application::sSceneManagerName.c_str());
        QScopedPointer<Scene> scene(new Scene(name,
                                              sceneFile,
                                              logicFile,
                                              sceneManager->getRootSceneNode(),
                                              engine,
                                              sceneMgrName));

        try
        {
            loadSceneLogic(scene.data(), logicFile);
        }
        catch(const std::runtime_error& ex)
        {
            qWarning("%s", ex.what());
            return NULL;
        }

        return scene.take();
    }
    else
    {
        qFatal("An initialized ogre engine and scene manager are required to load scenes.");
        return 0;
    }
}

void SceneLoader::loadSceneVisuals(OgreEngine* engine,
                                   Ogre::SceneManager* sceneManager,
                                   const QString& sceneFile)
{
    if(engine && sceneManager)
    {
        // Resources with textures must be loaded within Ogre's GL context
        engine->activateOgreContext();

        Ogre::DotSceneLoader dsloader;
        dsloader.parseDotScene(Ogre::String(sceneFile.toUtf8()), "General", sceneManager);

        engine->doneOgreContext();
    }
    else
    {
        qFatal("An initialized ogre engine and scene manager are required to load scenes.");
    }
}

void SceneLoader::loadSceneLogic(Scene* scene, const QString& logicFile)
{
    if(!scene)
    {
        throw std::runtime_error("A scene must be initialized.");
    }

    QFile file(logicFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        QString errorText(QString("Couldn't open logic file at %1.").arg(logicFile));
        throw std::runtime_error(errorText.toStdString());
    }

    QByteArray ba = file.readAll();
    file.close();

    QScriptEngine& engine = scene->getScriptEngine();

    JavaScriptBindings::addBindings(engine, scene);

    JavaScriptBindings::checkScriptEngineException(engine, "JS Bindings installation");

    qDebug("JS Bindings have been installed.");

    engine.evaluate(QScriptProgram(QString(ba), logicFile));

    JavaScriptBindings::checkScriptEngineException(engine, "Script evaluation");
}
