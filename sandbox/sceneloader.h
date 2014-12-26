#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <QString>
#include <QScriptEngine>

class Scene;
class OgreEngine;

namespace Ogre {
class SceneManager;
}

class SceneLoader
{
public:
    static Scene* loadScene(OgreEngine* engine,
                            Ogre::SceneManager* sceneManager,
                            const QString& name,
                            const QString& sceneFile,
                            const QString& logicFile);
private:
    static void loadSceneVisuals(OgreEngine* engine,
                                 Ogre::SceneManager* sceneManager,
                                 const QString& sceneFile);

    static void loadSceneLogic(Scene* scene,
                               const QString& logicFile);

    SceneLoader();
};

#endif // SCENELOADER_H
