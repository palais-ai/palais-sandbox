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
public:
    Actor(Ogre::SceneNode* node);

    Q_INVOKABLE void toggleHighlight(bool highlighted);

    const Ogre::Vector3& getPosition() const;
    void setPosition(const Ogre::Vector3& position);
    const Ogre::Quaternion& getRotation() const;
    void setRotation(const Ogre::Quaternion& rotation);

    QString getName() const;
    Q_INVOKABLE bool hasKnowledge(const QString& key) const;
    Q_INVOKABLE const QVariant& getKnowledge(const QString& key) const;
    Q_INVOKABLE void setKnowledge(const QString& key, const QVariant& value);

    QVariantMap& getKnowledge();
    void setKnowledge(const QVariantMap& map);
private:
    Ogre::SceneNode* mNode;
    QVariantMap mKnowledge;
};

#endif // ACTOR_H
