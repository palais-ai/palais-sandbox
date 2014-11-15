#ifndef SCENELOADER_H
#define SCENELOADER_H

#include <QString>

class Scene;

class SceneLoader
{
public:
    static Scene* loadScene(const QString& sceneFile, const QString& logicFile);
private:
    SceneLoader();
};

#endif // SCENELOADER_H
