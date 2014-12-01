#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <QVariant>
#include <QScriptEngine>

namespace Ogre {
class SceneNode;
}

class Scene
{
public:
    Scene(const QString& sceneFile, const QString& logicFile, Ogre::SceneNode* root);

    void update(float time);
    void performAction(const QString& actionName, const QVariant& params);
    QVariant getGlobalKnowledge(const QString& knowledgeKey) const;
    void setGlobalKnowledge(const QString& knowledgeKey, const QVariant& value);
    QVariant getActorKnowledge(const QString& actorName, const QString& knowledgeKey) const;
    void setActorKnowledge(const QString& actorName, const QString& knowledgeKey, const QVariant& value);

    QScriptEngine& getScriptEngine();
private:
    QString mSceneFile, mLogicFile;
    Ogre::SceneNode* mRoot;
    QScriptEngine mLogicScript;
};

#endif // SCENE_H
