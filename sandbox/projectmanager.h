#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "scenemanager.h"
#include <QObject>

class OgreEngine;

namespace Ogre {
class SceneManager;
}

class ProjectManager : public QObject
{
    Q_OBJECT
public:
    explicit ProjectManager(OgreEngine* engine, Ogre::SceneManager* sceneManager);

signals:
    void sceneLoaded();
    void sceneLoadFailed(const QString& errorMessage);
public slots:
    void onOpenProject(const QUrl& url);
private:
    SceneManager mScenarioManager;
};

#endif // PROJECTMANAGER_H
