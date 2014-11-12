#include "application.h"

#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>

#include "../libqmlogre/ogreitem.h"
#include "../libqmlogre/ogreengine.h"
#include "../libdotsceneloader/DotSceneLoader.h"

#include <QCoreApplication>
#include <QtQml/QQmlContext>
#include <Ogre.h>

Application::Application(QObject *parent) :
    QObject(parent),
    mOgreEngine(0),
    mSceneManager(0),
    mRoot(0),
    mApplicationEngine(0)
{
    ;
}

Application::~Application()
{
    if (mSceneManager)
    {
        mRoot->destroySceneManager(mSceneManager);
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
    mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

    // Resources with textures must be loaded within Ogre's GL context
    mOgreEngine->activateOgreContext();

    Ogre::DotSceneLoader dsloader;
    dsloader.parseDotScene("capturetheflag.scene", "General", mSceneManager);

    mOgreEngine->doneOgreContext();

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
