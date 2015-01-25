#include "application.h"
#include "projectmanager.h"
#include "scene.h"
#include "actor.h"
#include "models/inspectormodel.h"
#include "models/consolemodel.h"
#include "models/scenemodel.h"
#include "utility/timedlogger.h"
#include "utility/loghandler.h"

#include <cassert>

#include <QCoreApplication>
#include <QtQml/QQmlContext>
#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QTime>
#include <QDebug>
#include <QThread>

#include "../libqmlogre/ogreitem.h"
#include "../libqmlogre/ogreengine.h"

#include <Ogre.h>

Q_DECLARE_METATYPE(ConsoleModel::LogLevel)

const std::string Application::sSceneManagerName = "TheSceneManager";

static QWeakPointer<LogHandler> g_logHandler;
static void messageHandlerFun(QtMsgType type,
                              const QMessageLogContext& context,
                              const QString& msg)
{
    Q_UNUSED(context);

    ConsoleModel::LogLevel level;
    fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());

    switch (type)
    {
        case QtDebugMsg:
            level = ConsoleModel::LogLevelDebug;
            break;
        case QtWarningMsg:
            level = ConsoleModel::LogLevelWarning;
            break;
        case QtCriticalMsg:
            level = ConsoleModel::LogLevelError;
            break;
        case QtFatalMsg:
            level = ConsoleModel::LogLevelError;
            abort();
    }

    QSharedPointer<LogHandler> strongHandler = g_logHandler.toStrongRef();
    if(strongHandler)
    {
        strongHandler->broadcastNewMessageReceived(level, msg);
    }
}

Application::Application(QObject *parent) :
    QObject(parent),
    mApplicationEngine(NULL),
    mOgreEngine(NULL),
    mProjectManager(NULL),
    mRoot(NULL),
    mSceneModel(NULL),
    mInspectorModel(NULL),
    mConsoleModel(new ConsoleModel()),
    mTimeLogger(new TimedLogger()),
    mLogHandler(new LogHandler())
{
    qRegisterMetaType<ConsoleModel::LogLevel>();

    g_logHandler = mLogHandler;
    qInstallMessageHandler(messageHandlerFun);

    connect(mLogHandler.data(), &LogHandler::newMessageReceived,
            mConsoleModel.data(), &ConsoleModel::onMessageReceived);
}

Application::~Application()
{
    qInstallMessageHandler(0);

    if(mProjectManager)
    {
        delete mProjectManager;
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
    mTimeLogger->start();

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    mApplicationEngine = &engine;

    SceneModel::declareQML();
    ConsoleModel::declareQML();
    InspectorModel::declareQML();
    QQmlContext* rootCtx = mApplicationEngine->rootContext();
    rootCtx->setContextProperty("ConsoleModel", mConsoleModel.data());
    rootCtx->setContextProperty("ApplicationWrapper", this);
    mApplicationEngine->load(QUrl("qrc:/qml/MainWindow.qml"));

    QQuickWindow *window = qobject_cast<QQuickWindow *>(mApplicationEngine->rootObjects().first());

    window->setPersistentOpenGLContext(true);
    window->setPersistentSceneGraph(true);

    rootCtx->setContextProperty("ApplicationWindow", window);

    qmlRegisterType<Actor>();

    // start Ogre once we are in the rendering thread (Ogre must live in the rendering thread)
    connect(window, &QQuickWindow::frameSwapped,
            this, &Application::initializeOgre, Qt::DirectConnection);

    connect(this, &Application::ogreInitialized,
            this, &Application::onOgreIsReady, Qt::DirectConnection);

    connect(mApplicationEngine, &QQmlApplicationEngine::quit,
            &app, &QGuiApplication::quit);

    return app.exec();
}

// This function is executed in the OgreEngine's thread.
void Application::initializeOgre()
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(mApplicationEngine->rootObjects().first());

    // We only want to initialize once.
    disconnect(window, &QQuickWindow::frameSwapped,
               this, &Application::initializeOgre);

    TimedLogger engineStartupLogger;

    // Start up Ogre.
    mOgreEngine = new OgreEngine(window);

    mOgreEngine->startEngine();

    Ogre::LogManager::getSingleton().getDefaultLog()->addListener(mLogHandler.data());

    Ogre::MaterialManager& matMgr = Ogre::MaterialManager::getSingleton();
    matMgr.setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
    matMgr.setDefaultAnisotropy(16);

    mRoot = mOgreEngine->getRoot();
    mOgreEngine->setupResources();

    mProjectManager = new ProjectManager(mOgreEngine);
    mProjectManager->initializeSceneManager();

    engineStartupLogger.stop("Ogre3D startup");

    mApplicationEngine->rootContext()->setContextProperty("ProjectManager", mProjectManager);

    QObject::connect(mProjectManager, &ProjectManager::timePassed,
                     mConsoleModel.data(), &ConsoleModel::onTimePassed);

    QObject::connect(this, &Application::sceneSetupFinished,
                     mProjectManager, &ProjectManager::onSceneSetupFinished);

    QObject::connect(mProjectManager, &ProjectManager::onPlayingChanged,
                     this, &Application::onPlayingChanged);

    QObject::connect(this, &Application::selectActorAtClickpoint,
                     mProjectManager, &ProjectManager::onSelectActorAtClickpoint);

    QObject::connect(this, &Application::focusOnSelectedActor,
                     mProjectManager, &ProjectManager::onFocusSelectedActor);

    QObject::connect(mProjectManager, SIGNAL(sceneLoaded(Scene*)),
                     this, SLOT(onSceneLoaded(Scene*)));

    QObject::connect(mProjectManager, SIGNAL(sceneLoadFailed(QString)),
                     this, SLOT(onSceneLoadFailed(QString)));

    QObject::connect(mProjectManager, &ProjectManager::inspectorSelectionChanged,
                     this, &Application::onInspectorSelectionChanged);

    {
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
    }

    {
        QString dialogName("saveRenderingDialog");
        QObject* projectDialog = window->findChild<QObject*>(dialogName);

        if(projectDialog)
        {
            QObject::connect(projectDialog, SIGNAL(renderingFileSelected(QUrl)),
                             mProjectManager, SLOT(onSaveRenderView(QUrl)));
        }
        else
        {
            qFatal("Couldn't find rendering dialog (id=%s).", dialogName.toStdString().c_str());
        }
    }

    emit(ogreInitialized());
}

// This function is executed in the OgreEngine's thread.
void Application::onOgreIsReady()
{
    if(!mApplicationEngine)
    {
        qFatal("Application engine must be running when ogre is ready.");
        return;
    }

    mApplicationEngine->rootContext()->setContextProperty("OgreEngine", mOgreEngine);

    QMetaObject::invokeMethod(mApplicationEngine->rootObjects().first(), "onOgreIsReady");

    mTimeLogger->stop("Application Startup");
}

void Application::onOgreViewClicked(float mouseX, float mouseY)
{
    if(getSceneLoaded())
    {
        emit selectActorAtClickpoint(mouseX, mouseY);
    }
}

void Application::onSceneLoaded(Scene* scene)
{
    if(!scene)
    {
        qFatal("SceneLoaded signal was sent but scene wasn't instantiated.");
        return;
    }

    mInspectorModel.reset(new InspectorModel(scene->getName(), scene));

    mSceneModel.reset(new SceneModel(scene->getName()));

    connect(scene, &Scene::actorAdded,
            mSceneModel.data(), &SceneModel::onActorAdded);
    connect(scene, &Scene::actorRemoved,
            mSceneModel.data(), &SceneModel::onActorRemoved);
    connect(scene, &Scene::actorRemoved,
            mProjectManager, &ProjectManager::onActorRemoved);
    connect(scene, &Scene::actorChangedVisibility,
            mSceneModel.data(), &SceneModel::onActorChangedVisiblity);
    connect(mSceneModel.data(), &SceneModel::requestEmitCurrentActors,
            scene, &Scene::onRequestEmitCurrentActors);
    connect(mSceneModel.data(), &SceneModel::requestSelectionChange,
            mProjectManager, &ProjectManager::onActorChangeSelected);
    connect(mProjectManager, &ProjectManager::actorChangedSelected,
            mSceneModel.data(), &SceneModel::onActorChangedSelection);
    connect(mSceneModel.data(), &SceneModel::requestVisiblitiyChange,
            scene, &Scene::onActorChangeVisible);
    connect(scene, &Scene::actorChangedVisibility,
            mSceneModel.data(), &SceneModel::onActorChangedVisiblity);

    mSceneModel->requestCurrentActors();

    mApplicationEngine->rootContext()->setContextProperty("ActorModel",
                                                          mSceneModel.data());
    mApplicationEngine->rootContext()->setContextProperty("InspectorModel",
                                                          mInspectorModel.data());

    emit(sceneLoadedChanged(getSceneLoaded()));
    emit(sceneSetupFinished());
}

void Application::onInspectorSelectionChanged(const QString& name,
                                              const KnowledgeModel* knowledge)
{
    InspectorModel* oldModel = mInspectorModel.take();
    mInspectorModel.reset(new InspectorModel(name, knowledge));
    mApplicationEngine->rootContext()->setContextProperty("InspectorModel",
                                                          mInspectorModel.data());

    delete oldModel;
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
        QMetaObject::invokeMethod(mApplicationEngine->rootObjects().first(),
                                  "showErrorMessage",
                                  QGenericReturnArgument(),
                                  Q_ARG(QVariant, message));
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
    emit scenePlayingChanged(isPlaying);
}
