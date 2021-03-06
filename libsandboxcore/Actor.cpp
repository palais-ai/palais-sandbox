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

    //setMask(0);
    node->setListener(this);
    node->setUserAny(Ogre::Any(this));
}

Actor::~Actor()
{
    if(mNode)
    {
        mNode->setListener(NULL);
        mNode->setUserAny(Ogre::Any());
        mNode = NULL;
    }
}

Ogre::SceneNode* Actor::getSceneNode()
{
    return mNode;
}

void Actor::emitSignalBeforeRemoval()
{
    emit removedFromScene(this);
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
    assert(mNode);

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
    assert(mNode);
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
    assert(mNode);
    Ogre::SceneNode::ObjectIterator it = mNode->getAttachedObjectIterator();
    while(it.hasMoreElements())
    {
        Ogre::MovableObject* obj = static_cast<Ogre::MovableObject*>(it.getNext());
        obj->setCastShadows(hasShadows);
    }
}

bool Actor::isVisible() const
{
    assert(mNode);
    Ogre::SceneNode::ObjectIterator it = mNode->getAttachedObjectIterator();
    while(it.hasMoreElements())
    {
        Ogre::MovableObject* obj = static_cast<Ogre::MovableObject*>(it.getNext());
        if(!obj->isVisible())
        {
            return false;
        }
    }
    return true;
}

void Actor::setVisible(bool visible)
{
    assert(mNode);
    mNode->setVisible(visible);

    emit visibilityChanged(this, visible);
}

void Actor::update(float deltaTime)
{
    assert(mNode);
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

void Actor::nodeDestroyed(const Ogre::Node* node)
{
    Q_UNUSED(node);
    mNode = NULL;
}

const Ogre::Vector3& Actor::getPosition() const
{
    assert(mNode);
    return mNode->getPosition();
}

void Actor::setPosition(const Ogre::Vector3& position)
{
    assert(mNode);
    mNode->setPosition(position);
}

const Ogre::Quaternion& Actor::getRotation() const
{
    assert(mNode);
    return mNode->getOrientation();
}

void Actor::setRotation(const Ogre::Quaternion& rotation)
{
    assert(mNode);
    mNode->setOrientation(rotation);
}

const Ogre::Vector3& Actor::getScale() const
{
    assert(mNode);
    return mNode->getScale();
}

void Actor::setScale(const Ogre::Vector3& scale)
{
    assert(mNode);
    mNode->setScale(scale);
}

void Actor::setScale(float factor)
{
    assert(mNode);
    mNode->setScale(factor, factor, factor);
}

int Actor::getMask() const
{
    assert(mNode);
    if(mNode->numAttachedObjects() != 0)
    {
        return mNode->getAttachedObject(0)->getQueryFlags();
    }
    else
    {
        return 0;
    }
}
void Actor::setMask(int mask)
{
    assert(mNode);

    if(mNode->numAttachedObjects() != 0)
    {
        for(size_t i = 0; i < mNode->numAttachedObjects(); ++i)
        {
            mNode->getAttachedObject(i)->setQueryFlags(mask);
        }
    }
}

void Actor::toggleHighlight(bool highlighted)
{
    assert(mNode);
    mNode->showBoundingBox(highlighted);
}

QString Actor::getName() const
{
    assert(mNode);
    return QString::fromStdString(mNode->getName());
}

QString Actor::toString() const
{
    return "Actor (name: \"" + getName() + "\")";
}

void Actor::attach(Actor* other)
{
    assert(other);
    assert(other->mNode);
    assert(mNode);
    other->getSceneNode()->getParent()->removeChild(other->getSceneNode());
    mNode->addChild(other->getSceneNode());
}

void Actor::rotateBy(const Ogre::Vector3& axis, float angleDegrees)
{
    assert(mNode);
    mNode->rotate(axis, Ogre::Degree(angleDegrees));
}
