#ifndef SCENE_H
#define SCENE_H

#include <QString>
#include <QVariant>

class Scene
{
public:
    Scene(const QString& sceneFile, const QString& logicFile);

    void update(float time);
    void performAction(const QString& actionName, const QVariant& params);
    QVariant getGlobalKnowledge(const QString& knowledgeKey) const;
    void setGlobalKnowledge(const QString& knowledgeKey, const QVariant& value);
    QVariant getActorKnowledge(const QString& actorName, const QString& knowledgeKey) const;
    void setActorKnowledge(const QString& actorName, const QString& knowledgeKey, const QVariant& value);
private:
    QString mSceneFile, mLogicFile;
};

#endif // SCENE_H
