#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <QScriptEngine>

class Scene;
class QOEngine;
class PluginManager;

namespace Ogre
{
class SceneManager;
}

class SceneLoader
{
public:
    static Scene* loadScene(QOEngine* engine,
                            Ogre::SceneManager* sceneManager,
                            const QString& name,
                            const QString& sceneFile,
                            const QString& logicFile,
                            PluginManager& plugins
                            );
private:
    static void loadSceneVisuals(QOEngine* engine,
                                 Ogre::SceneManager* sceneManager,
                                 const QString& sceneFile);

    static void loadSceneLogic(Scene* scene,
                               const QString& logicFile,
                               PluginManager& plugins);

    SceneLoader();
};

#endif // SCENELOADER_H
