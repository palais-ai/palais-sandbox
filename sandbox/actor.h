#ifndef ACTOR_H
#define ACTOR_H

#include <QObject>
#include <QVariant>

#include <OgreVector3.h>
#include <OgreQuaternion.h>

namespace Ogre {
class SceneNode;
}

class Actor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(QVariantMap knowledge READ getKnowledge WRITE setKnowledge)
    Q_PROPERTY(Ogre::Vector3 position READ getPosition WRITE setPosition)
    Q_PROPERTY(Ogre::Quaternion rotation READ getRotation WRITE setRotation)
    Q_PROPERTY(Ogre::Vector3 scale READ getScale WRITE setScale)
    Q_PROPERTY(bool visible WRITE setVisible)
public:
    explicit Actor(Ogre::SceneNode* node);

    Q_INVOKABLE void toggleHighlight(bool highlighted);

    Ogre::SceneNode* getSceneNode();

    void update(float deltaTime);

    Q_INVOKABLE void enableAnimation(const QString& name);
    Q_INVOKABLE void disableAnimation(const QString& name);
    Q_INVOKABLE void setAnimationEnabled(const QString& name, bool enabled);
    Q_INVOKABLE void lookAt(const Ogre::Vector3& target);
    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();
    void setVisible(bool visible);

    const Ogre::Vector3& getPosition() const;
    void setPosition(const Ogre::Vector3& position);
    const Ogre::Quaternion& getRotation() const;
    void setRotation(const Ogre::Quaternion& rotation);
    const Ogre::Vector3& getScale() const;
    void setScale(const Ogre::Vector3& scale);
    Q_INVOKABLE void setScale(float factor);

    QString getName() const;
    Q_INVOKABLE bool hasKnowledge(const QString& key) const;
    Q_INVOKABLE QVariant getKnowledge(const QString& key) const;
    Q_INVOKABLE void removeKnowledge(const QString& key);
    Q_INVOKABLE void setKnowledge(const QString& key, const QVariant& value);
    Q_INVOKABLE QString toString() const;

    QVariantMap& getKnowledge();
    void setKnowledge(const QVariantMap& map);
private:
    Ogre::SceneNode* mNode;
    QVariantMap mKnowledge;
};

#endif // ACTOR_H
