#include "actor.h"

#include <cassert>

#include <OgreSceneNode.h>

Actor::Actor(Ogre::SceneNode* node) :
    mNode(node)
{
    if(!node)
    {
        qFatal("Node must be initialized.");
    }
}

void Actor::toggleHighlight()
{
    mNode->showBoundingBox(!mNode->getShowBoundingBox());
}

QString Actor::getName() const
{
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
