#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <QVariant>
#include <QScriptEngine>
#include <QMap>
#include <QAbstractListModel>

#include <OgreVector3.h>
#include <OgreQuaternion.h>

class Actor;
class OgreEngine;

namespace Ogre {
class SceneNode;
class RaySceneQuery;
}

class RaycastResult
{
public:
    Actor* actor;
    float distance;
};

class Scene : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(QVariantMap knowledge READ getKnowledge)
    Q_PROPERTY(QObjectList actors READ getActorsArray)
public:
    enum ModelRole {
        ModelRoleName = Qt::UserRole + 1,
        ModelRoleIndex
    };

    Scene(const QString& name, const QString& sceneFile, const QString& logicFile, Ogre::SceneNode* root, OgreEngine* ogreEngine);
    ~Scene();

    // List Model impl
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;

    Q_INVOKABLE void toggleHighlight(bool highlighted, int index);
    Q_INVOKABLE int size() const;

    Q_INVOKABLE Actor* instantiate(const QString& name,
                                   const QString& meshName,
                                   const Ogre::Vector3& position = Ogre::Vector3::ZERO,
                                   const Ogre::Quaternion& rotation = Ogre::Quaternion::IDENTITY,
                                   const Ogre::Vector3& scale = Ogre::Vector3(1,1,1));

    Q_INVOKABLE void destroy(Actor* actor);

    Q_INVOKABLE RaycastResult raycast(const Ogre::Vector3& origin, const Ogre::Vector3& direction); //< Reports the first hit actor in the scene

    void setup();
    void update(float time);
    void performAction(const QString& actionName, const QVariant& params = QVariant());

    Q_INVOKABLE bool hasKnowledge(const QString& knowledgeKey) const;
    Q_INVOKABLE QVariant getKnowledge(const QString& knowledgeKey) const;
    Q_INVOKABLE void setKnowledge(const QString& knowledgeKey, const QVariant& value);

    bool hasActorKnowledge(const QString& actorName, const QString& knowledgeKey) const;
    QVariant getActorKnowledge(const QString& actorName, const QString& knowledgeKey) const;
    void setActorKnowledge(const QString& actorName, const QString& knowledgeKey, const QVariant& value);

    QScriptEngine& getScriptEngine();
    const QMap<QString, Actor*>& getActors() const;
    Q_INVOKABLE Actor* getActor(unsigned int index);
    Q_INVOKABLE QObjectList getActorsArray() const;
    QVariantMap& getKnowledge();
    const QString& getName() const;
private:
    void getActors(Ogre::SceneNode* root);
    Actor* getActorForNode(Ogre::SceneNode* node) const;
    void destroyAllAttachedMovableObjects( Ogre::SceneNode* i_pSceneNode );

    QString mName, mSceneFile, mLogicFile;
    Ogre::SceneNode* mRoot;
    OgreEngine* mEngine;
    Ogre::RaySceneQuery* mRayQuery;
    QScriptEngine mLogicScript;
    QMap<QString, Actor*> mActors;
    QVariantMap mKnowledge;
    bool mIsSetup;
};

#endif // SCENE_H
