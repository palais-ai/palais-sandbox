#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <QVariant>
#include <QScriptEngine>
#include <QMap>
#include <QAbstractListModel>

class Actor;

namespace Ogre {
class SceneNode;
}

class Scene : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
public:
    enum ModelRole {
        ModelRoleName = Qt::UserRole + 1,
        ModelRoleIndex
    };

    Scene(const QString& name, const QString& sceneFile, const QString& logicFile, Ogre::SceneNode* root);
    ~Scene();

    // List Model impl
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;

    Q_INVOKABLE void toggleHighlight(int index);
    Q_INVOKABLE int size() const;

    void setup();
    void update(float time);
    void performAction(const QString& actionName, const QVariant& params);
    QVariant getGlobalKnowledge(const QString& knowledgeKey) const;
    void setGlobalKnowledge(const QString& knowledgeKey, const QVariant& value);
    QVariant getActorKnowledge(const QString& actorName, const QString& knowledgeKey) const;
    void setActorKnowledge(const QString& actorName, const QString& knowledgeKey, const QVariant& value);

    QScriptEngine& getScriptEngine();
    const QMap<QString, Actor*>& getActors() const;
    const QString& getName() const;
private:
    void getActors(Ogre::SceneNode* root);
    void destroyAllAttachedMovableObjects( Ogre::SceneNode* i_pSceneNode );

    QString mName, mSceneFile, mLogicFile;
    Ogre::SceneNode* mRoot;
    QScriptEngine mLogicScript;
    QMap<QString, Actor*> mActors;
};

#endif // SCENE_H
