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
protected:
    void notifySuccess();
    void notifyFailure();
private:
    BehaviorListener* mListener;
};

class Composite : public Behavior, public BehaviorListener
{
public:
    typedef std::vector<Behavior*> ChildList;

    Composite(const ChildList& children);

    const ChildList& getChildren() const;
private:
    ChildList mChildren;
};

class Selector : public Composite
{
public:
    Selector(const Composite::ChildList& children);

    virtual void run();
    virtual void onSuccess(Behavior* behavior);
    virtual void onFailure(Behavior* behavior);
};

class Sequence : public Composite
{
public:
    Sequence(const Composite::ChildList& children);

    virtual void run();
    virtual void onSuccess(Behavior* behavior);
    virtual void onFailure(Behavior* behavior);
};

class Parallel : public Composite
{
public:
    Parallel(Scheduler& scheduler, const Composite::ChildList& children);

    virtual void run();
    virtual void onSuccess(Behavior* behavior);
    virtual void onFailure(Behavior* behavior);

    void enqueue(Behavior* behavior);
    void dequeue(Behavior* behavior);
private:
    Scheduler& mScheduler;
    uint32_t mSuccessCount;
};

END_NS_AILIB

#endif // BEHAVIORTREE_H
