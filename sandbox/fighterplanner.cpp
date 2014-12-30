#include "fighterplanner.h"

FighterPlanner::FighterPlanner()
{
}

void FighterPlanner::initialize(const OgreHelper::NavigationGraph& navGraph,
                                const Ogre::Vector3& enemyFlagPosition,
                                const Ogre::Vector3& ownFlagPosition)
{
    mPlanner.addAction(new MoveToEnemyFlagAction(enemyFlagPosition, navGraph));
}
