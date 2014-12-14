#include "application.h"
#include "projectmanager.h"
#include "scene.h"
#include "actor.h"

#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>

#include "../libqmlogre/ogreitem.h"
#include "../libqmlogre/ogreengine.h"
#include "../libqmlogre/cameranodeobject.h"

#include <QCoreApplication>
#include <QtQml/QQmlContext>
#include <Ogre.h>

const std::string Application::sSceneManagerName = "TheSceneManager";

Application::Application(QObject *parent) :
    QObject(parent),
    mApplicationEngine(NULL),
    mOgreEngine(NULL),
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

    window->setPersistentOpenGLContext(true);
    window->setPersistentSceneGraph(true);

    mApplicationEngine->rootContext()->setContextProperty("ApplicationWindow", window);

    qmlRegisterType<Actor>();

    // start Ogre once we are in the rendering thread (Ogre must live in the rendering thread)
    connect(window, &QQuickWindow::frameSwapped, this, &Application::initializeOgre, Qt::DirectConnection);
    connect(this, &Application::ogreInitialized, this, &Application::onOgreIsReady, Qt::DirectConnection);
    connect(mApplicationEngine, &QQmlApplicationEngine::quit, &app, &QGuiApplication::quit);

    return app.exec();
}

void Application::initializeSceneManager()
{
    mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC, sSceneManagerName);
    mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

    // This fixes some issues with ray casting when using shallow terrain.
    Ogre::AxisAlignedBox box;
    Ogre::Vector3 max(100000, 100000, 100000);
    box.setExtents(-max, max);
    mSceneManager->setOption("Size", &box);
}

void Application::initializeOgre()
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(mApplicationEngine->rootObjects().first());

    // we only want to initialize once
    disconnect(window, &QQuickWindow::frameSwapped, this, &Application::initializeOgre);

    // start up Ogre
    mOgreEngine = new OgreEngine(window);

    mOgreEngine->startEngine();
    mRoot = mOgreEngine->getRoot();
    mOgreEngine->setupResources();

    initializeSceneManager();

    mProjectManager = new ProjectManager(mOgreEngine);

    mApplicationEngine->rootContext()->setContextProperty("ProjectManager", mProjectManager);

    QObject::connect(mProjectManager, &ProjectManager::onPlayingChanged,
                     this, &Application::onPlayingChanged);

    QObject::connect(mProjectManager, SIGNAL(sceneLoaded(Scene*)),
                     this, SLOT(onSceneLoaded(Scene*)));

    QObject::connect(mProjectManager, SIGNAL(sceneLoadFailed(QString)),
                     this, SLOT(onSceneLoadFailed(QString)));

    QObject::connect(mProjectManager, SIGNAL(beforeSceneLoad(QString, QString, QString)),
                     this, SLOT(onBeforeSceneLoad(QString, QString, QString)));

    QObject::connect(this, SIGNAL(beforeSceneLoadFinished(QString, QString, QString)),
                     mProjectManager, SLOT(onBeforeSceneLoadFinished(QString,QString,QString)));

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
    if(!mApplicationEngine)
    {
        qFatal("Application engine must be running when ogre is ready.");
        return;
    }

    mApplicationEngine->rootContext()->setContextProperty("OgreEngine", mOgreEngine);

    QMetaObject::invokeMethod(mApplicationEngine->rootObjects().first(), "onOgreIsReady");
}

void Application::onOgreViewClicked(float mouseX, float mouseY)
{
    if(getSceneLoaded()) {
        mProjectManager->selectActorAtClickpoint(mouseX, mouseY, getCameraWithName("cam1")->camera());
    }
}

void Application::onBeforeSceneLoad(const QString& name, const QString& sceneFile, const QString& logicFile)
{
    qDebug("Before scene load");

    mRoot = mOgreEngine->getRoot();

    if(!mRoot)
    {
        qFatal("An Ogre Root must be instantiated before scene load.");
    }

    mOgreEngine->lockEngine();

    if(mSceneManager)
    {
        mRoot->destroySceneManager(mSceneManager);
    }

    initializeSceneManager();

    CameraNodeObject* camera = getCameraWithName("cam1");
    camera->createCameraWithCurrentSceneManager();

    mOgreEngine->unlockEngine();

    emit(beforeSceneLoadFinished(name, sceneFile, logicFile));
}

CameraNodeObject* Application::getCameraWithName(const QString& cameraName)
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(mApplicationEngine->rootObjects().first());
    CameraNodeObject* camera = window->findChild<CameraNodeObject*>(cameraName);

    if(!camera)
    {
        qFatal("Couldn't find camera with name (objectName=%s).", cameraName.toStdString().c_str());
    }

    return camera;
}

void Application::onSceneLoaded(Scene* scene)
{
    if(!mSceneManager)
    {
        qFatal("Scene manager must be instantiated when a scene finished loading.");
        return;
    }

    if(!scene)
    {
        qFatal("SceneLoaded signal was sent but scene wasn't instantiated.");
        return;
    }

    QString cameraName("cam1");
    CameraNodeObject* camera = getCameraWithName(cameraName);

    if(!camera)
    {
        qFatal("Couldn't find first camera (id=%s).", cameraName.toStdString().c_str());
    }

    mOgreEngine->lockEngine();
    camera->fitToContain(mSceneManager->getRootSceneNode());
    mOgreEngine->unlockEngine();

    // We need a proxy model, since the scene resides in the ogre thread, but updates must be made in the GUI thread.
    mActorProxyModel.setSourceModel(scene);
    mApplicationEngine->rootContext()->setContextProperty("ActorModel", &mActorProxyModel);
    mApplicationEngine->rootContext()->setContextProperty("Scene", scene);
    emit(onSceneLoadedChanged(true));
}

bool Application::getSceneLoaded() const
{
    return mProjectManager ? mProjectManager->getSceneLoaded() : false;
}

bool Application::getScenePlaying() const
{
    return mProjectManager ? mProjectManager->isPlaying() : false;
}

void Application::onSceneLoadFailed(const QString& message)
{
    if(mApplicationEngine)
    {
        QMetaObject::invokeMethod(mApplicationEngine->rootObjects().first(), "showErrorMessage", QGenericReturnArgument(), Q_ARG(QVariant, message));
    }
    else
    {
        qWarning("Application engine should already be running when a scene load fails.");
    }
}

void Application::onPlayButtonPressed()
{
    if(!mProjectManager)
    {
        qWarning("ProjectManager must've been instantiated to start playing a scene.");
        return;
    }

    if(!mProjectManager->getSceneLoaded())
    {
        qWarning("The play button was pressed, despite no scene being loaded.");
        return;
    }

    if(getScenePlaying())
    {
        mProjectManager->pause();
    }
    else
    {
        mProjectManager->play();
    }
}

void Application::onPlayingChanged(bool isPlaying)
{
    emit onScenePlayingChanged(isPlaying);
}

