#ifndef ACTOR_H
#define ACTOR_H

#include "KnowledgeModel.h"
#include <OgreVector3.h>
#include <OgreQuaternion.h>

namespace Ogre
{
class SceneNode;
}

class DLL_EXPORT Actor : public KnowledgeModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(Ogre::Vector3 position READ getPosition WRITE setPosition)
    Q_PROPERTY(Ogre::Quaternion rotation READ getRotation WRITE setRotation)
    Q_PROPERTY(Ogre::Vector3 scale READ getScale WRITE setScale)
    Q_PROPERTY(bool visible WRITE setVisible)
public:
    explicit Actor(Ogre::SceneNode* node);

    void update(float deltaTime);

    Q_INVOKABLE void toggleHighlight(bool highlighted);
    Q_INVOKABLE void setScale(float factor);
    Q_INVOKABLE void enableAnimation(const QString& name);
    Q_INVOKABLE void disableAnimation(const QString& name);
    Q_INVOKABLE void setAnimationEnabled(const QString& name, bool enabled);
    Q_INVOKABLE void lookAt(const Ogre::Vector3& target);
    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();
    Q_INVOKABLE void setCastShadows(bool hasShadows);
    Q_INVOKABLE QString toString() const;
    Q_INVOKABLE void attach(Actor* other);

    void setVisible(bool visible);

    const Ogre::Vector3& getPosition() const;
    void setPosition(const Ogre::Vector3& position);
    const Ogre::Quaternion& getRotation() const;
    void setRotation(const Ogre::Quaternion& rotation);
    const Ogre::Vector3& getScale() const;
    void setScale(const Ogre::Vector3& scale);

    QString getName() const;
    Ogre::SceneNode* getSceneNode();
signals:
    void visibilityChanged(Actor* self, bool visible);
    void removedFromScene(Actor* self);
private:
    Ogre::SceneNode* mNode;
};

#endif // ACTOR_H
