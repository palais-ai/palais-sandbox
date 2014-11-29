#include "application.h"
#include "scenemanager.h"

#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>

#include "../libqmlogre/ogreitem.h"
#include "../libqmlogre/ogreengine.h"

#include <QCoreApplication>
#include <QtQml/QQmlContext>
#include <Ogre.h>

Application::Application(QObject *parent) :
    QObject(parent),
    mOgreEngine(NULL),
    mSceneManager(NULL),
    mApplicationEngine(NULL),
    mScenarioManager(NULL),
    mRoot(NULL)
{
    ;
}

Application::~Application()
{
    if (mSceneManager)
    {
        mRoot->destroySceneManager(mSceneManager);
    }

    if(mRoot)
    {
        delete mRoot;
    }

    if(mOgreEngine)
    {
        delete mOgreEngine;
    }
}

int Application::onApplicationStarted(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    mApplicationEngine = &engine;

    mApplicationEngine->rootContext()->setContextProperty("ApplicationWrapper", this);
    mApplicationEngine->load(QUrl("qrc:/qml/MainWindow.qml"));

    QQuickWindow *window = qobject_cast<QQuickWindow *>(mApplicationEngine->rootObjects().first());

    mApplicationEngine->rootContext()->setContextProperty("ApplicationWindow", window);

    // start Ogre once we are in the rendering thread (Ogre must live in the rendering thread)
    connect(window, &QQuickWindow::frameSwapped, this, &Application::initializeOgre, Qt::DirectConnection);
    connect(this, &Application::ogreInitialized, this, &Application::onOgreIsReady, Qt::DirectConnection);
    connect(mApplicationEngine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    return app.exec();
}

void Application::initializeOgre()
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(mApplicationEngine->rootObjects().first());

    // we only want to initialize once
    disconnect(window, &QQuickWindow::frameSwapped, this, &Application::initializeOgre);

    // start up Ogre
    mOgreEngine = new OgreEngine(window);

    connect(mOgreEngine, &OgreEngine::onLoadingProgressChanged, this, &Application::onLoadingProgressChanged);

    mRoot = mOgreEngine->startEngine();
    mOgreEngine->setupResources();

    // set up Ogre scene
    mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC, "mySceneManager");
    mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_NONE);

    mScenarioManager = new SceneManager(mOgreEngine, mSceneManager);
    mScenarioManager->loadScene("capturetheflag.scene", "ctf.js");

    emit(ogreInitialized());
}

void Application::onOgreIsReady()
{
    mApplicationEngine->rootContext()->setContextProperty("OgreEngine", mOgreEngine);

    QMetaObject::invokeMethod(mApplicationEngine->rootObjects().first(), "onOgreIsReady");
}

qreal Application::loadingProgress() const
{
    return mOgreEngine ? mOgreEngine->loadingProgress() : 0;
}
