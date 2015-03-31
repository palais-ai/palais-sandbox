#include "Actor.h"
#include <cassert>
#include <QDebug>
#include <QVector>
#include <OgreSceneNode.h>
#include <OgreAnimation.h>
#include <OgreEntity.h>

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
    if(mNode->numAttachedObjects() == 0)
    {
        qWarning() << "Can't enable animation [" << name
                   << "] on actor [" << getName() << "]"
                   << " because it doesn't have any attached objects.";
        return;
    }

    Ogre::Entity* entity = dynamic_cast<Ogre::Entity*>(mNode->getAttachedObject(0));

    if(!entity)
    {
        qWarning() << "Can't enable animation [" << name
                   << "] on actor [" << getName() << "]"
                   << " because its attachment isn't an entity.";
        return;
    }

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

    state->setEnabled(enabled);
    state->setLoop(true);
}

void Actor::lookAt(const Ogre::Vector3& target)
{
    mNode->lookAt(target, Ogre::Node::TS_WORLD);
}

void Actor::show()
{
    setVisible(true);
}

void Actor::hide()
{
    setVisible(false);
}

void Actor::setCastShadows(bool hasShadows)
{
    Ogre::SceneNode::ObjectIterator it = mNode->getAttachedObjectIterator();
    while(it.hasMoreElements())
    {
        Ogre::MovableObject* obj = static_cast<Ogre::MovableObject*>(it.getNext());
        obj->setCastShadows(hasShadows);
    }
}

void Actor::setVisible(bool visible)
{
    mNode->setVisible(visible);

    emit visibilityChanged(this, visible);
}

void Actor::update(float deltaTime)
{
    if(mNode->numAttachedObjects() == 0)
    {
        return;
    }

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
            state->addTime(deltaTime);
        }
    }
}

const Ogre::Vector3& Actor::getPosition() const
{
    return mNode->getPosition();
}

void Actor::setPosition(const Ogre::Vector3& position)
{
    mNode->setPosition(position);
}

const Ogre::Quaternion& Actor::getRotation() const
{
    return mNode->getOrientation();
}

void Actor::setRotation(const Ogre::Quaternion& rotation)
{
    mNode->setOrientation(rotation);
}

const Ogre::Vector3& Actor::getScale() const
{
    return mNode->getScale();
}

void Actor::setScale(const Ogre::Vector3& scale)
{
    mNode->setScale(scale);
}

void Actor::setScale(float factor)
{
    mNode->setScale(factor, factor, factor);
}

void Actor::toggleHighlight(bool highlighted)
{
    mNode->showBoundingBox(highlighted);
}

QString Actor::getName() const
{
    return QString::fromStdString(mNode->getName());
}

QString Actor::toString() const
{
    return "Actor (name: \"" + getName() + "\")";
}

void Actor::attach(Actor* other)
{
    other->getSceneNode()->getParent()->removeChild(other->getSceneNode());
    mNode->addChild(other->getSceneNode());
}
