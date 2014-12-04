#include "actor.h"

#include <cassert>

#include <QMutexLocker>

#include <OgreSceneNode.h>

extern QMutex g_engineMutex;

Actor::Actor(Ogre::SceneNode* node) :
    mNode(node)
{
    if(!node)
    {
        qFatal("Node must be initialized.");
    }
}

const Ogre::Vector3& Actor::getPosition() const
{
    QMutexLocker locker(&g_engineMutex);

    return mNode->getPosition();
}

void Actor::setPosition(const Ogre::Vector3& position)
{
    QMutexLocker locker(&g_engineMutex);

    mNode->setPosition(position);
}

const Ogre::Quaternion& Actor::getRotation() const
{
    QMutexLocker locker(&g_engineMutex);

    return mNode->getOrientation();
}

void Actor::setRotation(const Ogre::Quaternion& rotation)
{
    QMutexLocker locker(&g_engineMutex);

    mNode->setOrientation(rotation);
}

void Actor::toggleHighlight(bool highlighted)
{
    QMutexLocker locker(&g_engineMutex);

    mNode->showBoundingBox(highlighted);
}

QString Actor::getName() const
{
    QMutexLocker locker(&g_engineMutex);

    return mNode->getName().c_str();
}

const QVariant& Actor::getKnowledge(const QString& key) const
{
    return mKnowledge[key];
}

void Actor::setKnowledge(const QString& key, const QVariant& value)
{
    mKnowledge[key] = value;
}

QVariantMap& Actor::getKnowledge()
{
    return mKnowledge;
}

void Actor::setKnowledge(const QVariantMap& map)
{
    mKnowledge = map;
}
