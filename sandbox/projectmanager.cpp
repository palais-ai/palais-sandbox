#include "projectmanager.h"
#include "Scene.h"
#include "Actor.h"
#include "application.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QFileInfo>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreCamera.h>
#include "QOEngine.h"
#include "QOCamera.h"
#include "QOItem.h"

std::string ProjectManager::sCurrentResourceGroupName = "CurrentScene";

ProjectManager::ProjectManager(QOEngine* engine) :
    QObject(0),
    mScenarioManager(engine),
    mSelectedActor(NULL)
{
    assert(thread() == engine->thread());

    connect(&mScenarioManager, &SceneManager::timePassed,
            this, &ProjectManager::onTimePassed);
    connect(this, &ProjectManager::signalReloadProject,
            this, &ProjectManager::onReloadProject);
    connect(this, &ProjectManager::signalSetSimulationSpeed,
            this, &ProjectManager::onSetSimulationSpeed);
    connect(this, &ProjectManager::signalUnselectActor,
            this, &ProjectManager::onUnselectActor);

    startTimer(1000);
}

ProjectManager::~ProjectManager()
{
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
    mgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
    mgr->setAmbientLight(Ogre::ColourValue(1, 1, 1));
    mgr->setShadowCasterRenderBackFaces(false);
    mgr->setCameraRelativeRendering(true);
    mgr->setShowDebugShadows(true);

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
        qWarning("Can't focus selected actor, because none is selected.");
        return;
    }

    QOCamera* cameraNode = getCameraWithName("cam1");
    if(!cameraNode)
    {
        qWarning("Can't focus selected actor without a corresponding CameraNode.");
        return;
    }

    cameraNode->focus(mSelectedActor->getSceneNode());
}

void ProjectManager::onSaveRenderView(const QUrl& url)
{
    assert(QThread::currentThread() == thread());

    QString itemName("qoItem");
    QQuickWindow* window = mScenarioManager.getEngine()->getQQuickWindow();
    QOItem* item = window->findChild<QOItem*>(itemName);

    if(!item)
    {
        qFatal("Couldn't find ogre item with name (objectName=%s).",
               itemName.toLocal8Bit().constData());
    }

    QImage img = item->saveCurrentImage();

    if(img.isNull())
    {
        qWarning("The screenshot couldn't be allocated. Out of memory.");
        return;
    }

    QString path = url.toLocalFile();
    if(!img.save(path + ".png", "PNG", 100))
    {
        qWarning("Failed to save current scene screenshot to %s", path.toLocal8Bit().constData());
    }
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

void ProjectManager::unselectActor()
{
    // Queue signal to call on the engine thread.
    emit signalUnselectActor();
}

void ProjectManager::onUnselectActor()
{
    if(mSelectedActor)
    {
        emit onActorChangeSelected(mSelectedActor->getName(), false);
    }
}

void ProjectManager::onActorRemoved(const QString& actorName)
{
    QOCamera* cameraNode = getCameraWithName("cam1");
    if(!cameraNode)
    {
        // A crash is immanent if this happens anyway - Exit through qFatal.
        qFatal("Can't determine focused actor without a corresponding CameraNode.");
    }
    else
    {
        // Refocus the camera on the root scene node.
        if(cameraNode->focusedNode() ==
           mScenarioManager.getCurrentScene()->getActorByName(actorName)->getSceneNode())
        {
            Ogre::Root& root = Ogre::Root::getSingleton();
            Ogre::SceneManager* mgr = root.getSceneManager(Application::sSceneManagerName);
            cameraNode->focus(mgr->getRootSceneNode());
        }
    }

    if(mSelectedActor && mSelectedActor->getName() == actorName)
    {
        Scene* current = mScenarioManager.getCurrentScene();
        changeInspectorSelection(current->getName(),
                                 current->getKnowledge(),
                                 current);
        mSelectedActor = NULL;
    }
}

void ProjectManager::timerEvent(QTimerEvent *)
{
    emit oneSecondTimeout();
}

void ProjectManager::onTimePassed(const QTime& time)
{
    emit timePassed(time);
}

void ProjectManager::onSelectActorAtClickpoint(float mouseX,
                                               float mouseY)
{
    assert(QThread::currentThread() == thread());

    QOCamera* cameraNode = getCameraWithName("cam1");
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
    QSharedPointer<Actor> hitActor = current->raycast(mouseRay.getOrigin(),
                                                      mouseRay.getDirection()).actor.toStrongRef();

    if(hitActor)
    {
        onActorChangeSelected(hitActor->getName(),
                              !hitActor->getSceneNode()->getShowBoundingBox());
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
        changeInspectorSelection(current->getName(), current->getKnowledge(), current);
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
        changeInspectorSelection(mSelectedActor->getName(),
                                 mSelectedActor->getKnowledge(),
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

    emit playingChanged(isPlaying());
}

void ProjectManager::pause()
{
    mScenarioManager.pause();

    emit playingChanged(isPlaying());
}

void ProjectManager::onSetSimulationSpeed(float speedFactor)
{
    assert(QThread::currentThread() == thread());

    mScenarioManager.setSimulationSpeed(speedFactor);
}

// FIXME: Move the parsing code to a separate class
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
        QString errorMsg = QString("Failed to parse the project file at %0, because it isn't a JSON document.")
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
        emit(sceneLoadFailed(QString("Failed to load the project file at %0, because it is missing the mandatory property %1.")
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
        emit(sceneLoadFailed(QString("Failed to load the project file at %0, because it is missing the mandatory property %1.")
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
        emit(sceneLoadFailed(QString("Failed to load the project file at %0, because it is missing the mandatory property %1.")
                             .arg(url.toLocalFile())
                             .arg(namePropertyName)));
        return;
    }

    QString resourcesPropertyName("resources");
    QStringList resources;
    if(obj.contains(resourcesPropertyName))
    {
        QJsonArray array = obj[resourcesPropertyName].toArray();
        foreach(QJsonValue val, array)
        {
            resources << (url.adjusted(QUrl::RemoveFilename).toLocalFile() + val.toString());
        }
    }


    const bool isReopen = mCurrentProjectUrl == url;
    mLastOpenedUrl = url;
    mCurrentProjectUrl.clear();

    pause();
    mScenarioManager.unloadCurrentScene();
    mSelectedActor = NULL;

    Ogre::ResourceGroupManager& rgm = Ogre::ResourceGroupManager::getSingleton();
    if(!isReopen && rgm.resourceGroupExists(sCurrentResourceGroupName))
    {
        Ogre::StringVectorPtr locs = rgm.findResourceLocation(sCurrentResourceGroupName, "*");

        foreach(std::string loc, *locs)
        {
            rgm.removeResourceLocation(loc);
        }

        rgm.destroyResourceGroup(sCurrentResourceGroupName);
    }

    QString cameraName("cam1");
    QOCamera* camera = getCameraWithName(cameraName);

    if(!camera)
    {
        QString msg = QString("Couldn't find first camera (id=%1).").arg(cameraName);
        emit sceneLoadFailed(msg);
        return;
    }

    prepareScene(camera);
    loadResources(resources);

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

void ProjectManager::onSceneSetupFinished()
{
    mCurrentProjectUrl = mLastOpenedUrl;
}

void ProjectManager::loadResources(const QStringList& paths)
{
    Ogre::ResourceGroupManager& rgm = Ogre::ResourceGroupManager::getSingleton();
    foreach(QString path, paths)
    {
        QFileInfo info(path);

        if(info.exists())
        {
            rgm.addResourceLocation(path.toStdString(),
                                    "FileSystem",
                                    sCurrentResourceGroupName);
            qDebug() << "Loaded resource location at [" << path << "].";
        }
        else
        {
            qWarning() << "Couldn't add resource location ["
                       << path << "] because it doesn't exist.";
        }
    }

    rgm.initialiseResourceGroup(sCurrentResourceGroupName);
}

void ProjectManager::prepareScene(QOCamera* camera)
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

QOCamera* ProjectManager::getCameraWithName(const QString& cameraName)
{
    QQuickWindow *window = mScenarioManager.getEngine()->getQQuickWindow();
    QOCamera* camera = window->findChild<QOCamera*>(cameraName);

    if(!camera)
    {
        qFatal("Couldn't find camera with name (objectName=%s).",
               cameraName.toLocal8Bit().constData());
    }

    return camera;
}

void ProjectManager::changeInspectorSelection(QString name,
                                              QVariantMap data,
                                              const KnowledgeModel* model)
{
    emit connectKnowledgeModel(model);
    emit inspectorSelectionChanged(name, data);
}
