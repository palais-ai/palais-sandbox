#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

namespace Ogre {
    class SceneManager;
    class Root;
}

class OgreEngine;
class QQmlApplicationEngine;

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = nullptr);
    ~Application();

    int onApplicationStarted(int argc, char **argv);

signals:
    void ogreInitialized();

public slots:
    void initializeOgre();
    void onOgreIsReady();

private:
    QQmlApplicationEngine* mApplicationEngine;
    OgreEngine *mOgreEngine;
    Ogre::SceneManager *mSceneManager;
    Ogre::Root *mRoot;
};

#endif // APPLICATION_H
