#include "projectmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QDir>

#include <OgreRoot.h>
#include <OgreSceneManager.h>

#include "../libqmlogre/ogreengine.h"

ProjectManager::ProjectManager(OgreEngine* engine) :
    QObject(0),
    mScenarioManager(engine),
    mKnowledgeService(mScenarioManager),
    mActorService(mScenarioManager)
{
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
        qDebug() << "Local RPC service listening on " << serviceName << ".";
    }
    else
    {
        qWarning() << "Couldn't start the sandbox service on " << serviceName << ", because " << mServer.errorString();
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

void ProjectManager::reloadProject()
{
    onOpenProject(mCurrentProjectUrl);
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
}

void ProjectManager::pause()
{
    mScenarioManager.pause();
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
        emit(sceneLoadFailed(QString("Couldn't open project file at %1.").arg(url.toLocalFile())));
        return;
    }

    QByteArray ba = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(ba, &error);

    if(error.error != QJsonParseError::NoError)
    {
        emit(sceneLoadFailed(QString("Failed to parse JSON file, because: %1.").arg(error.errorString())));
        return;
    }

    QJsonObject obj = doc.object();

    QString visualPropertyName("visual");
    QString sceneFile;
    if(obj.contains(visualPropertyName))
    {
        sceneFile = url.adjusted(QUrl::RemoveFilename).toLocalFile() + obj[visualPropertyName].toString();
    }
    else
    {
        emit(sceneLoadFailed(QString("The project file was missing the property %1.").arg(visualPropertyName)));
        return;
    }

    QString logicPropertyName("logic");
    QString logicFile;
    if(obj.contains(logicPropertyName))
    {
        qDebug("logic file path: %s, adjusted: %s", url.toLocalFile().toStdString().c_str(), url.adjusted(QUrl::RemoveFilename).toLocalFile().toStdString().c_str());
        logicFile = url.adjusted(QUrl::RemoveFilename).toLocalFile() + obj[logicPropertyName].toString();
    }
    else
    {
        emit(sceneLoadFailed(QString("The project file was missing the property %1.").arg(logicPropertyName)));
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
        emit(sceneLoadFailed(QString("The project file was missing the property %1.").arg(namePropertyName)));
        return;
    }

    mLastOpenedUrl = url;

    emit(beforeSceneLoad(name, sceneFile, logicFile));
}

void ProjectManager::onBeforeSceneLoadFinished(const QString& name, const QString& sceneFile, const QString& logicFile)
{
    qDebug("Loading project %s with visuals (%s) and logic (%s).", name.toStdString().c_str(), sceneFile.toStdString().c_str(), logicFile.toStdString().c_str());

    Scene* scene = mScenarioManager.loadScene(name, sceneFile, logicFile);

    if(!scene)
    {
        emit(sceneLoadFailed(QString("Failed to load scene %1.").arg(sceneFile)));
        return;
    }

    mCurrentProjectUrl = mLastOpenedUrl;

    emit(sceneLoaded(scene));
}
