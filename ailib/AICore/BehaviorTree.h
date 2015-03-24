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
    virtual ~BehaviorListener() {}
    virtual void onSuccess(Behavior* behavior) = 0;
    virtual void onFailure(Behavior* behavior) = 0;
    virtual void onReset(Behavior* behavior);
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
    void notifyReset();
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
    uint32_t indexOf(const Behavior* child) const;

    Scheduler& mScheduler;
};

class SequentialComposite : public Composite
{
public:
    SequentialComposite(Scheduler& scheduler,
                        const Composite::BehaviorList& children);

    virtual void run();
    virtual void terminate();
    virtual void onReset(Behavior* behavior);
protected:
    bool indexIsCurrent(uint32_t idx) const;
    bool currentIsLastBehavior() const;
    void scheduleNextBehavior();
    void terminateFromIndex(uint32_t idx);
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
    virtual void onReset(Behavior* behavior);
private:
    void resetCodes();
    bool allChildrenSucceeded() const;
    bool anyChildrenFailed() const;

    enum ReturnCode
    {
        ReturnCodeNone = 0,
        ReturnCodeSuccess,
        ReturnCodeFailure
    };
    static const int32_t sMaxChildCount = 8;
    ReturnCode mCodes[sMaxChildCount];
};

class Decorator : public Behavior, public BehaviorListener
{
public:
    Decorator(Scheduler& scheduler, Behavior* child);

    virtual void terminate();
    virtual void onSuccess(Behavior* behavior);
    virtual void onFailure(Behavior* behavior);
    virtual void onReset(Behavior* behavior);

    const Behavior* getChild() const;
protected:
    void scheduleBehavior();
private:
    Scheduler& mScheduler;
    Behavior* const mChild;
};

// TODO: BlackboardScope, random sequence / selector

END_NS_AILIB

#endif // BEHAVIORTREE_H
