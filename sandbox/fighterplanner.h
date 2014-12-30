#ifndef FIGHTERPLANNER_H
#define FIGHTERPLANNER_H

#include "ogrehelper.h"

#include "GOAP.h"
#include "AStar.h"

#include <bitset>

#include <OgreVector3.h>

class FighterState
{
public:
    static FighterState makeState(const Ogre::Vector3& position,
                                  bool hasEnemyInSight,
                                  bool hasAmmo,
                                  bool isGettingShotAt,
                                  bool hasFlag,
                                  bool isLowHealth,
                                  bool isEnemyDead,
                                  bool isInDefensivePosition)
    {
        FighterState state;

        state.position = position;
        state.set(0, hasEnemyInSight);
        state.set(1, hasAmmo);
        state.set(2, isGettingShotAt);
        state.set(3, hasFlag);
        state.set(4, isLowHealth);
        state.set(5, isEnemyDead);
        state.set(6, isInDefensivePosition);

        return state;
    }

    FORCE_INLINE bool hasEnemyInSight() const
    {
        return mBitset.test(0);
    }

    FORCE_INLINE bool hasAmmo() const
    {
        return mBitset.test(1);
    }

    FORCE_INLINE bool isGettingShotAt() const
    {
        return mBitset.test(2);
    }

    FORCE_INLINE bool hasFlag() const
    {
        return mBitset.test(3);
    }

    FORCE_INLINE bool isLowHealth() const
    {
        return mBitset.test(4);
    }

    FORCE_INLINE bool isEnemyDead() const
    {
        return mBitset.test(5);
    }

    FORCE_INLINE void set(size_t pos, bool value)
    {
        mBitset.set(pos, value);
    }

    Ogre::Vector3 position;

private:
    std::bitset<8> mBitset;
};

class MoveToAction : public ailib::Action<FighterState>
{
public:
    MoveToAction(const Ogre::Vector3& target,
                 const OgreHelper::NavigationGraph& navGraph) :
        mTarget(target),
        mNavGraph(navGraph)
    {
        ;
    }

    virtual bool isPreconditionFulfilled(const FighterState& state) const
    {
        // Target must be reachable
        return OgreHelper::planPath(mNavGraph, state.position, mTarget).size() != 0;
    }

    virtual float getCost(const FighterState& state) const
    {
        return state.position.distance(mTarget);
    }

private:
    Ogre::Vector3 mTarget;
    const OgreHelper::NavigationGraph& mNavGraph;
};

class MoveToEnemyFlagAction : public MoveToAction
{
public:
    MoveToEnemyFlagAction(const Ogre::Vector3& target,
                          const OgreHelper::NavigationGraph& navGraph) :
        MoveToAction(target, navGraph)
    {
        ;
    }

    virtual void applyPostcondition(FighterState& state)
    {
        state.set(3, true);
    }
};

class MoveToOwnFlagAction : public MoveToAction
{
public:
    MoveToOwnFlagAction(const Ogre::Vector3& target,
                        const OgreHelper::NavigationGraph& navGraph) :
        MoveToAction(target, navGraph)
    {
        ;
    }

    virtual void applyPostcondition(FighterState& state)
    {
        state.set(3, false);
    }
};

class MoveToDefensivePositionAction : public MoveToAction
{
public:
    MoveToDefensivePositionAction(const Ogre::Vector3& target,
                                  const OgreHelper::NavigationGraph& navGraph) :
        MoveToAction(target, navGraph)
    {
        ;
    }

    virtual void applyPostcondition(FighterState& state)
    {
        state.set(3, false);
    }
};

class ShootEnemyAction : public ailib::Action<FighterState>
{
    virtual bool isPreconditionFulfilled(const FighterState& state) const = 0;
    virtual void applyPostcondition(FighterState& state) = 0;
    virtual float getCost(const FighterState& state) const
    {
        return state.isLowHealth() ? 20 : 10.f;
    }
};

class FighterPlanner
{
public:
    FighterPlanner();

    void initialize(const OgreHelper::NavigationGraph& navGraph,
                    const Ogre::Vector3& enemyFlagPosition,
                    const Ogre::Vector3& ownFlagPosition);
private:
    ailib::GOAPPlanner<FighterState> mPlanner;
};

#endif // FIGHTERPLANNER_H
