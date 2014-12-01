#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

class ProjectManager;

namespace Ogre {
class SceneManager;
class Root;
}

class OgreEngine;
class QQmlApplicationEngine;

class Application : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal loadingProgress READ loadingProgress NOTIFY onLoadingProgressChanged)
public:
    explicit Application(QObject *parent = 0);
    ~Application();

    int onApplicationStarted(int argc, char **argv);
    qreal loadingProgress() const;
signals:
    void ogreInitialized();
    void onLoadingProgressChanged(qreal progress);
public slots:
    void initializeOgre();
    void onOgreIsReady();
    void onSceneLoaded();
    void onSceneLoadFailed(const QString& message);
private:
    QQmlApplicationEngine* mApplicationEngine;
    OgreEngine *mOgreEngine;
    Ogre::SceneManager *mSceneManager;
    ProjectManager* mProjectManager;
    Ogre::Root *mRoot;
};

#endif // APPLICATION_H
