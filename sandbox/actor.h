#ifndef ACTOR_H
#define ACTOR_H

#include <QObject>
#include <QVariant>

namespace Ogre {
class SceneNode;
}

class Actor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
public:
    Actor(Ogre::SceneNode* node);

    void toggleHighlight(bool highlighted);

    QString getName() const;
    const QVariant& getKnowledge(const QString& key) const;
    void setKnowledge(const QString& key, const QVariant& value);
private:
    Ogre::SceneNode* mNode;
    QVariantMap mKnowledge;
};

#endif // ACTOR_H
