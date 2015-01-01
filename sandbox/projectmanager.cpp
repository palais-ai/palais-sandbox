#include "projectmanager.h"
#include "scene.h"
#include "actor.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QDir>

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreCamera.h>

#include "../libqmlogre/ogreengine.h"

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
}

void ProjectManager::onTimePassed(const QTime& time)
{
    emit timePassed(time);
}

void ProjectManager::selectActorAtClickpoint(float mouseX,
                                             float mouseY,
                                             Ogre::Camera* camera)
{
    if(!camera)
    {
        qWarning("Cant determine an actor to select without a corresponding camera.");
        return;
    }

    qDebug() << "mouse x: " << mouseX << ", mouse y: " << mouseY;

    Ogre::Ray mouseRay = camera->getCameraToViewportRay(mouseX, mouseY);

    Actor* oldSelected = mSelectedActor;
    mSelectedActor = mScenarioManager.getCurrentScene()->raycast(mouseRay.getOrigin(),
                                                                 mouseRay.getDirection()).actor;

    if(mSelectedActor)
    {
        qDebug() << "Clicked " << mSelectedActor->getName();

        if(oldSelected && oldSelected != mSelectedActor)
        {
            oldSelected->toggleHighlight(false);
            emit actorChangedSelected(oldSelected->getName(), false);
        }

        const bool newState = !mSelectedActor->getSceneNode()->getShowBoundingBox();

        mSelectedActor->toggleHighlight(newState);
        emit actorChangedSelected(mSelectedActor->getName(),
                                  newState);
        emit inspectorSelectionChanged(mSelectedActor->getName(),
                                       mSelectedActor->getKnowledge());
    }
    else
    {
        qDebug() << "No hit.";
    }
}

void ProjectManager::onActorChangeSelected(const QString& actorName,
                                           bool selected)
{
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
        emit inspectorSelectionChanged(current->getName(),
                                       current->getKnowledge());
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
                                       mSelectedActor->getKnowledge());
    }
}

void ProjectManager::reloadProject()
{
    if(getSceneLoaded())
    {
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

void ProjectManager::setSimulationSpeed(float speedFactor)
{
    mScenarioManager.setSimulationSpeed(speedFactor);
}

void ProjectManager::onOpenProject(const QUrl& url)
{
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

    mCurrentProjectUrl.clear();
    mLastOpenedUrl = url;

    pause();
    mScenarioManager.unloadCurrentScene();

    emit(beforeSceneLoad(name, sceneFile, logicFile));
}

void ProjectManager::onBeforeSceneLoadFinished(const QString& name,
                                               const QString& sceneFile,
                                               const QString& logicFile)
{
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

    mCurrentProjectUrl = mLastOpenedUrl;

    emit(sceneLoaded(scene));
}
