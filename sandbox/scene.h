#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <QVariant>
#include <QScriptEngine>
#include <QMap>
#include <QAbstractListModel>
#include <QVector3D>
#include <QQuaternion>

class Actor;
class OgreEngine;

namespace Ogre {
class SceneNode;
}

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

    Q_INVOKABLE Actor* instantiate(const QString& name, Actor* prototype, const QVector3D& position, const QQuaternion& rotation);
    Q_INVOKABLE void destroy(Actor* actor);

    void setup();
    void update(float time);
    void performAction(const QString& actionName, const QVariant& params);

    Q_INVOKABLE bool hasKnowledge(const QString& knowledgeKey) const;
    Q_INVOKABLE QVariant getKnowledge(const QString& knowledgeKey) const;
    Q_INVOKABLE void setKnowledge(const QString& knowledgeKey, const QVariant& value);
    QVariant getActorKnowledge(const QString& actorName, const QString& knowledgeKey) const;
    void setActorKnowledge(const QString& actorName, const QString& knowledgeKey, const QVariant& value);

    QScriptEngine& getScriptEngine();
    const QMap<QString, Actor*>& getActors() const;
    Q_INVOKABLE QObjectList getActorsArray() const;
    QVariantMap& getKnowledge();
    const QString& getName() const;
    void checkScriptEngineException(const QString& context = QString());
private:
    void getActors(Ogre::SceneNode* root);
    void destroyAllAttachedMovableObjects( Ogre::SceneNode* i_pSceneNode );

    QString mName, mSceneFile, mLogicFile;
    Ogre::SceneNode* mRoot;
    OgreEngine* mEngine;
    QScriptEngine mLogicScript;
    QMap<QString, Actor*> mActors;
    QVariantMap mKnowledge;
};

#endif // SCENE_H
