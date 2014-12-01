#include "application.h"
#include "projectmanager.h"

#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>

#include "../libqmlogre/ogreitem.h"
#include "../libqmlogre/ogreengine.h"
#include "../libqmlogre/cameranodeobject.h"

#include <QCoreApplication>
#include <QtQml/QQmlContext>
#include <Ogre.h>

Application::Application(QObject *parent) :
    QObject(parent),
    mOgreEngine(NULL),
    mApplicationEngine(NULL),
    mSceneManager(NULL),
    mProjectManager(NULL),
    mRoot(NULL)
{
    ;
}

Application::~Application()
{
    if(mProjectManager)
    {
        delete mProjectManager;
    }

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

    mProjectManager = new ProjectManager(mOgreEngine, mSceneManager);

    mApplicationEngine->rootContext()->setContextProperty("ProjectManager", mProjectManager);

    QObject::connect(mProjectManager, SIGNAL(sceneLoaded()),
                     this, SLOT(onSceneLoaded()));

    QObject::connect(mProjectManager, SIGNAL(sceneLoadFailed(QString)),
                     this, SLOT(onSceneLoadFailed(QString)));

    QString dialogName("openProjectDialog");
    QObject* projectDialog = window->findChild<QObject*>(dialogName);

    if(projectDialog)
    {
        QObject::connect(projectDialog, SIGNAL(projectFileSelected(QUrl)),
                         mProjectManager, SLOT(onOpenProject(QUrl)));
    }
    else
    {
        qFatal("Couldn't find project dialog (id=%s).", dialogName.toStdString().c_str());
    }

    emit(ogreInitialized());
}

void Application::onOgreIsReady()
{
    mApplicationEngine->rootContext()->setContextProperty("OgreEngine", mOgreEngine);

    QMetaObject::invokeMethod(mApplicationEngine->rootObjects().first(), "onOgreIsReady");
}

void Application::onSceneLoaded()
{
    if(!mSceneManager)
    {
        return;
    }

    QString cameraName("cam1");
    QQuickWindow *window = qobject_cast<QQuickWindow *>(mApplicationEngine->rootObjects().first());
    CameraNodeObject* camera = window->findChild<CameraNodeObject*>(cameraName);

    if(!camera)
    {
        qFatal("Couldn't find first camera (id=%s).", cameraName.toStdString().c_str());
    }

    mOgreEngine->lockEngine();
    camera->fitToContain(mSceneManager->getRootSceneNode());
    mOgreEngine->unlockEngine();
}

qreal Application::loadingProgress() const
{
    return mOgreEngine ? mOgreEngine->loadingProgress() : 0;
}

void Application::onSceneLoadFailed(const QString& message)
{
    QMetaObject::invokeMethod(mApplicationEngine->rootObjects().first(), "showErrorMessage", QGenericReturnArgument(), Q_ARG(QVariant, message));
}

