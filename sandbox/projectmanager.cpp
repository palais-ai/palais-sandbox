#include "projectmanager.h"
#include "scene.h"
#include "actor.h"
#include "application.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QThread>

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreCamera.h>

#include "../libqmlogre/ogreengine.h"
#include "../libqmlogre/cameranodeobject.h"

ProjectManager::ProjectManager(OgreEngine* engine) :
    QObject(0),
    mScenarioManager(engine),
    mKnowledgeService(mScenarioManager),
    mActorService(mScenarioManager),
    mSelectedActor(NULL)
{
    assert(thread() == engine->thread());

    connect(&mScenarioManager, &SceneManager::timePassed,
            this, &ProjectManager::onTimePassed);
    connect(this, &ProjectManager::signalReloadProject,
            this, &ProjectManager::onReloadProject);
    connect(this, &ProjectManager::signalSetSimulationSpeed,
            this, &ProjectManager::onSetSimulationSpeed);

    QString serviceName = QDir::temp().absoluteFilePath("SandboxService");
    if (QFile::exists(serviceName))
    {
        if (!QFile::remove(serviceName))
        {
            qWarning() << "Couldn't delete local temporary service file.";
        }
    }

    mServer.addService(&mKnowledgeService);
    mServer.addService(&mActorService);
    if (mServer.listen(serviceName))
    {
        qDebug() << "Local RPC service listening on "
                 << serviceName << ".";
    }
    else
    {
        qWarning() << "Couldn't start the sandbox service on "
                   << serviceName
                   << ", because "
                   << mServer.errorString();
        mServer.close();
    }
}

ProjectManager::~ProjectManager()
{
    if(!mServer.removeService(&mKnowledgeService) ||
       !mServer.removeService(&mActorService))
    {
        qWarning() << "Failed to remove the sandbox services on close.";
    }

    Ogre::Root& root = Ogre::Root::getSingleton();
    if(root.hasSceneManager(Application::sSceneManagerName))
    {
        Ogre::SceneManager* mgr = root.getSceneManager(Application::sSceneManagerName);
        root.destroySceneManager(mgr);
    }
}

void ProjectManager::initializeSceneManager()
{
    assert(QThread::currentThread() == thread());

    Ogre::Root& root = Ogre::Root::getSingleton();
    Ogre::SceneManager* mgr = NULL;
    if(root.hasSceneManager(Application::sSceneManagerName))
    {
        mgr = root.getSceneManager(Application::sSceneManagerName);
        root.destroySceneManager(mgr);
    }

    mgr = root.createSceneManager(Ogre::ST_GENERIC, Application::sSceneManagerName);
    mgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);
    mgr->setAmbientLight(Ogre::ColourValue(1, 1, 1));

    // This fixes some issues with ray casting when using shallow terrain.
    Ogre::AxisAlignedBox box;
    Ogre::Vector3 max(100000, 100000, 100000);
    box.setExtents(-max, max);
    mgr->setOption("Size", &box);
}

void ProjectManager::onFocusSelectedActor()
{
    if(!mSelectedActor)
    {
        qWarning("Cant focus selected actor, because none is selected.");
        return;
    }

    CameraNodeObject* cameraNode = getCameraWithName("cam1");
    if(!cameraNode)
    {
        qWarning("Cant focus selected actor without a corresponding CameraNode.");
        return;
    }

    cameraNode->focus(mSelectedActor->getSceneNode());
}

void ProjectManager::setSimulationSpeed(float speedFactor)
{
    // Queue signal to call on the engine thread.
    emit signalSetSimulationSpeed(speedFactor);
}

void ProjectManager::reloadProject()
{
    // Queue signal to call on the engine thread.
    emit signalReloadProject();
}

void ProjectManager::onTimePassed(const QTime& time)
{
    emit timePassed(time);
}

void ProjectManager::onSelectActorAtClickpoint(float mouseX,
                                               float mouseY)
{
    assert(QThread::currentThread() == thread());


    CameraNodeObject* cameraNode = getCameraWithName("cam1");
    if(!cameraNode)
    {
        qWarning("Cant determine an actor to select without a corresponding CameraNode.");
        return;
    }

    Ogre::Camera* camera = cameraNode->camera();
    if(!camera)
    {
        qWarning("Cant determine an actor to select without a corresponding ogre camera.");
        return;
    }

    Ogre::Ray mouseRay = camera->getCameraToViewportRay(mouseX, mouseY);

    Scene* current = mScenarioManager.getCurrentScene();
    Actor* hitActor = current->raycast(mouseRay.getOrigin(),
                                       mouseRay.getDirection()).actor;

    if(hitActor)
    {
        qDebug() << "Clicked " << hitActor->getName();

        onActorChangeSelected(hitActor->getName(),
                              !hitActor->getSceneNode()->getShowBoundingBox());
    }
    else
    {
        qDebug() << "No hit.";
    }
}

void ProjectManager::onActorChangeSelected(const QString& actorName,
                                           bool selected)
{
    assert(QThread::currentThread() == thread());

    Scene* current = mScenarioManager.getCurrentScene();
    Actor* newSelected = current->getActor(actorName);

    if(!newSelected)
    {
        qWarning("Can't select actor %s, because its not part of the scene.",
                 actorName.toLocal8Bit().constData());
        return;
    }

    newSelected->toggleHighlight(selected);
    if(newSelected == mSelectedActor && !selected)
    {
        emit actorChangedSelected(mSelectedActor->getName(), false);
        emit inspectorSelectionChanged(current->getName(), current);
        mSelectedActor = NULL;
    }
    else if(selected)
    {
        if(mSelectedActor)
        {
            mSelectedActor->toggleHighlight(false);
            emit actorChangedSelected(mSelectedActor->getName(), false);
        }

        mSelectedActor = newSelected;
        emit actorChangedSelected(mSelectedActor->getName(), true);
        emit inspectorSelectionChanged(mSelectedActor->getName(),
                                       mSelectedActor);
    }
}

void ProjectManager::onReloadProject()
{
    assert(QThread::currentThread() == thread());
    if(!mCurrentProjectUrl.isEmpty())
    {
        qDebug() << "Reloading " << mCurrentProjectUrl;
        onOpenProject(mCurrentProjectUrl);
    }
}

bool ProjectManager::getSceneLoaded() const
{
    return mScenarioManager.getCurrentScene() != NULL;
}

bool ProjectManager::isPlaying() const
{
    return mScenarioManager.isPlaying();
}

void ProjectManager::play()
{
    mScenarioManager.start();

    emit onPlayingChanged(isPlaying());
}

void ProjectManager::pause()
{
    mScenarioManager.pause();

    emit onPlayingChanged(isPlaying());
}

void ProjectManager::onSetSimulationSpeed(float speedFactor)
{
    assert(QThread::currentThread() == thread());

    mScenarioManager.setSimulationSpeed(speedFactor);
}

void ProjectManager::onOpenProject(const QUrl url)
{
    assert(QThread::currentThread() == thread());

    QFile file(url.toLocalFile());

    if(!file.open(QIODevice::ReadOnly))
    {
        QString errorMsg = QString("Failed to open the project file at %1.")
                                    .arg(url.toLocalFile());

        emit(sceneLoadFailed(errorMsg));
        return;
    }

    QByteArray ba = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(ba, &error);

    if(error.error != QJsonParseError::NoError)
    {
        QString errorMsg = QString("Failed to parse the project file at %0,\
                                    because it isn't a JSON document.")
                                    .arg(url.toLocalFile());
        emit(sceneLoadFailed(errorMsg));
        return;
    }

    QJsonObject obj = doc.object();

    QString visualPropertyName("visual");
    QString sceneFile;
    if(obj.contains(visualPropertyName))
    {
        sceneFile = url.adjusted(QUrl::RemoveFilename).toLocalFile()
                    + obj[visualPropertyName].toString();
    }
    else
    {
        emit(sceneLoadFailed(QString("Failed to load the project file at %0,\
                                      because it is missing the mandatory property %1.")
                             .arg(url.toLocalFile())
                             .arg(visualPropertyName)));
        return;
    }

    QString logicPropertyName("logic");
    QString logicFile;
    if(obj.contains(logicPropertyName))
    {
        qDebug("logic file path: %s, adjusted: %s",
               url.toLocalFile().toStdString().c_str(),
               url.adjusted(QUrl::RemoveFilename).toLocalFile().toStdString().c_str());

        logicFile = url.adjusted(QUrl::RemoveFilename).toLocalFile()
                    + obj[logicPropertyName].toString();
    }
    else
    {
        emit(sceneLoadFailed(QString("Failed to load the project file at %0,\
                                      because it is missing the mandatory property %1.")
                             .arg(url.toLocalFile())
                             .arg(logicPropertyName)));
        return;
    }

    QString namePropertyName("name");
    QString name;
    if(obj.contains(namePropertyName))
    {
        name = obj[namePropertyName].toString();
    }
    else
    {
        emit(sceneLoadFailed(QString("Failed to load the project file at %0,\
                                      because it is missing the mandatory property %1.")
                             .arg(url.toLocalFile())
                             .arg(namePropertyName)));
        return;
    }


    mLastOpenedUrl = url;
    mCurrentProjectUrl.clear();

    pause();
    mScenarioManager.unloadCurrentScene();
    mSelectedActor = NULL;

    QString cameraName("cam1");
    CameraNodeObject* camera = getCameraWithName(cameraName);

    if(!camera)
    {
        QString msg = QString("Couldn't find first camera (id=%1).").arg(cameraName);
        emit sceneLoadFailed(msg);
        return;
    }

    prepareScene(camera);

    qDebug("Loading project %s with visuals (%s) and logic (%s).",
           name.toStdString().c_str(),
           sceneFile.toStdString().c_str(),
           logicFile.toStdString().c_str());

    Scene* scene = mScenarioManager.loadScene(name, sceneFile, logicFile);

    if(!scene)
    {
        emit(sceneLoadFailed(QString("Failed to load scene %1.")
                                     .arg(sceneFile)));
        return;
    }

    Ogre::Root& root = Ogre::Root::getSingleton();
    camera->focus(root.getSceneManager(Application::sSceneManagerName)
                             ->getRootSceneNode());

    emit(sceneLoaded(scene));
}

void ProjectManager::prepareScene(CameraNodeObject* camera)
{
    if(!camera)
    {
        qFatal("Need a camera to prepare scene.");
        return;
    }

    Ogre::Root* root = Ogre::Root::getSingletonPtr();

    if(!root)
    {
        qFatal("An Ogre Root must be instantiated before scene load.");
    }

    initializeSceneManager();

    // Update current camera to use the new scene manager.
    camera->createCameraWithCurrentSceneManager();
}

CameraNodeObject* ProjectManager::getCameraWithName(const QString& cameraName)
{
    QQuickWindow *window = mScenarioManager.getOgreEngine()->getQQuickWindow();
    CameraNodeObject* camera = window->findChild<CameraNodeObject*>(cameraName);

    if(!camera)
    {
        qFatal("Couldn't find camera with name (objectName=%s).",
               cameraName.toLocal8Bit().constData());
    }

    return camera;
}

void ProjectManager::onSceneSetupFinished()
{
    mCurrentProjectUrl = mLastOpenedUrl;
}
