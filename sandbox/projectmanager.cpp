#include "projectmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QString>

#include "../libqmlogre/ogreengine.h"

ProjectManager::ProjectManager(OgreEngine* engine, Ogre::SceneManager* sceneManager) :
    QObject(0),
    mScenarioManager(engine, sceneManager)
{
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
        sceneFile = url.adjusted(QUrl::RemoveFilename).path() + obj[visualPropertyName].toString();
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
        logicFile = url.adjusted(QUrl::RemoveFilename).path() + obj[logicPropertyName].toString();
    }
    else
    {
        emit(sceneLoadFailed(QString("The project file was missing the property %1.").arg(logicPropertyName)));
        return;
    }

    qDebug("Loading project with visuals (%s) and logic (%s).", sceneFile.toStdString().c_str(), logicFile.toStdString().c_str());

    Scene* scene = mScenarioManager.loadScene(sceneFile, logicFile);

    if(!scene)
    {
        emit(sceneLoadFailed(QString("Failed to load scene %1.").arg(sceneFile)));
        return;
    }

    emit(sceneLoaded());
}
