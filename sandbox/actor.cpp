#include "actor.h"

#include <cassert>

#include <QMutexLocker>
#include <QDebug>

#include <OgreSceneNode.h>
#include <OgreAnimation.h>
#include <OgreEntity.h>

extern QMutex g_engineMutex;

Actor::Actor(Ogre::SceneNode* node) :
    mNode(node)
{
    if(!node)
    {
        qFatal("Node must be initialized.");
    }
}

Ogre::SceneNode* Actor::getSceneNode()
{
    return mNode;
}

void Actor::enableAnimation(const QString& name)
{
    setAnimationEnabled(name, true);
}

void Actor::disableAnimation(const QString& name)
{
    setAnimationEnabled(name, false);
}

void Actor::setAnimationEnabled(const QString& name, bool enabled)
{
    QMutexLocker locker(&g_engineMutex);

    Ogre::Entity* entity = static_cast<Ogre::Entity*>(mNode->getAttachedObject(0));

    Ogre::AnimationStateSet* set = entity->getAllAnimationStates();

    if(!set->hasAnimationState(name.toStdString()))
    {
        qWarning() << "Tried to set the state of an animation named "
                   << name << " on actor "
                   << QString::fromStdString(mNode->getName())
                   << " that doesn't exist. The animation states remain unchanged.";
        return;
    }

    Ogre::AnimationState* state = set->getAnimationState(name.toStdString());

    state->setWeight(1);
    state->setEnabled(enabled);
    state->setLoop(true);
}

void Actor::lookAt(const Ogre::Vector3& target)
{
    QMutexLocker locker(&g_engineMutex);

    mNode->lookAt(target, Ogre::Node::TS_WORLD);
}

void Actor::update(float deltaTime)
{
    QMutexLocker locker(&g_engineMutex);

    Ogre::Entity* entity = dynamic_cast<Ogre::Entity*>(mNode->getAttachedObject(0));

    if(!entity)
    {
        return;
    }

    Ogre::AnimationStateSet* set = entity->getAllAnimationStates();
    if(!set)
    {
        return;
    }

    Ogre::AnimationStateIterator it = set->getAnimationStateIterator();
    while(it.hasMoreElements())
    {
        Ogre::AnimationState* state = it.getNext();

        if(state->getEnabled())
        {
            //qDebug() << "Updating animation " << QString::fromStdString(state->getAnimationName()) << " by " << deltaTime << " seconds. Current time is " << state->getTimePosition();
            state->addTime(deltaTime);
        }
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

const Ogre::Vector3& Actor::getScale() const
{
    QMutexLocker locker(&g_engineMutex);

    return mNode->getScale();
}

void Actor::setScale(const Ogre::Vector3& scale)
{
    QMutexLocker locker(&g_engineMutex);

    mNode->setScale(scale);
}

void Actor::setScale(float factor)
{
    QMutexLocker locker(&g_engineMutex);
    mNode->setScale(factor, factor, factor);
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


bool Actor::hasKnowledge(const QString& key) const
{
    return mKnowledge.contains(key);
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

QString Actor::toString() const
{
    return "Actor (name: \"" + getName() + "\")";
}
