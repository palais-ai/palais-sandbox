#ifndef BEHAVIORTREE_H
#define BEHAVIORTREE_H

#pragma once

#include "ai_global.h"
#include "Scheduler.h"
#include <vector>

BEGIN_NS_AILIB

class Behavior;

class BehaviorListener
{
public:
    virtual void onSuccess(Behavior* behavior) = 0;
    virtual void onFailure(Behavior* behavior) = 0;
};

class Behavior : public Task
{
public:
    Behavior();
    virtual ~Behavior();

    void setListener(BehaviorListener* listener);

    // __terminate__ should be called by parent nodes that
    // forcefully remove this behavior from the scheduler.
    virtual void terminate();
protected:
    void notifySuccess();
    void notifyFailure();
private:
    BehaviorListener* mListener;
};

class Composite : public Behavior, public BehaviorListener
{
public:
    typedef std::vector<Behavior*> BehaviorList;

    Composite(Scheduler& scheduler, const BehaviorList& children);

    const BehaviorList& getChildren() const;
private:
    BehaviorList mChildren;
protected:
    Scheduler& mScheduler;
};

class SequentialComposite : public Composite
{
public:
    SequentialComposite(Scheduler& scheduler,
                        const Composite::BehaviorList& children);

    virtual void run();
    virtual void terminate();
protected:
    bool currentIsLastBehavior() const;
    void scheduleNextBehavior();
private:
    uint16_t mCurrentBehavior;
};

class Selector : public SequentialComposite
{
public:
    Selector(Scheduler& scheduler,
             const Composite::BehaviorList& children);

    virtual void onSuccess(Behavior* behavior);
    virtual void onFailure(Behavior* behavior);
};

class Sequence : public SequentialComposite
{
public:
    Sequence(Scheduler& scheduler,
             const Composite::BehaviorList& children);

    virtual void onSuccess(Behavior* behavior);
    virtual void onFailure(Behavior* behavior);
};

class Parallel : public Composite
{
public:
    Parallel(Scheduler& scheduler,
             const Composite::BehaviorList& children);

    virtual void run();
    virtual void terminate();
    virtual void onSuccess(Behavior* behavior);
    virtual void onFailure(Behavior* behavior);
private:
    uint16_t mSuccessCount;
};

class Decorator : public Behavior, public BehaviorListener
{
public:
    Decorator(Scheduler& scheduler, Behavior* child);
private:
    Scheduler& mScheduler;
    Behavior* const mChild;
};

// TODO: BlackboardScope, random sequence / selector

END_NS_AILIB

#endif // BEHAVIORTREE_H
