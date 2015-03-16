#include "BehaviorTree.h"
#include <cassert>

BEGIN_NS_AILIB

Behavior::Behavior() :
    mListener(0)
{
    ;
}

Behavior::~Behavior()
{
    ;
}

void Behavior::setListener(BehaviorListener* listener)
{
    mListener = listener;
}

void Behavior::terminate()
{
    ;
}

void Behavior::notifySuccess()
{
    setStatus(StatusDone);

    if(mListener)
    {
        mListener->onSuccess(this);
    }
}

void Behavior::notifyFailure()
{
    setStatus(StatusDone);

    if(mListener)
    {
        mListener->onFailure(this);
    }
}

Composite::Composite(Scheduler& scheduler,
                     const Composite::BehaviorList& children) :
    mChildren(children),
    mScheduler(scheduler)
{
    for(Composite::BehaviorList::iterator it = mChildren.begin();
        it != mChildren.end(); ++it)
    {
        (*it)->setListener(this);
    }
}

const Composite::BehaviorList& Composite::getChildren() const
{
    return mChildren;
}

SequentialComposite::SequentialComposite(Scheduler& scheduler,
                                         const Composite::BehaviorList& children) :
    Composite(scheduler, children),
    mCurrentBehavior(0)
{
    ;
}

void SequentialComposite::run()
{
    // The composite has no children.
    if(getChildren().empty())
    {
        notifySuccess();
    }
    // Start by scheduling the first child for execution.
    else
    {
        mScheduler.enqueue(getChildren()[mCurrentBehavior]);
        setStatus(StatusWaiting);
    }
}

void SequentialComposite::terminate()
{
    if(getStatus() == StatusWaiting)
    {
        getChildren()[mCurrentBehavior]->terminate();
        mScheduler.dequeue(getChildren()[mCurrentBehavior]);
    }
}

bool SequentialComposite::currentIsLastBehavior() const
{
    // Ensure no overflow for signed / unsigned comparison.
    assert(getChildren().size() <= std::numeric_limits<int32_t>::max());

    return mCurrentBehavior + 1 == static_cast<int32_t>(getChildren().size());
}

void SequentialComposite::scheduleNextBehavior()
{
    mScheduler.enqueue(getChildren()[++mCurrentBehavior]);
}

Selector::Selector(Scheduler& scheduler,
                   const Composite::BehaviorList& children) :
    SequentialComposite(scheduler, children)
{
    ;
}

void Selector::onSuccess(Behavior* behavior)
{
    UNUSED(behavior);

    notifySuccess();
}

void Selector::onFailure(Behavior* behavior)
{
    UNUSED(behavior);

    // The successful behavior was the last one in the sequence.
    if(currentIsLastBehavior())
    {
        notifyFailure();
    }
    // Schedule the next behavior in the sequence.
    else
    {
        scheduleNextBehavior();
    }
}

Sequence::Sequence(Scheduler& scheduler,
                   const Composite::BehaviorList& children) :
    SequentialComposite(scheduler, children)
{
    ;
}

void Sequence::onSuccess(Behavior* behavior)
{
    UNUSED(behavior);

    if(currentIsLastBehavior())
    {
        notifySuccess();
    }
    else
    {
        scheduleNextBehavior();
    }
}

void Sequence::onFailure(Behavior* behavior)
{
    UNUSED(behavior);

    notifyFailure();
}

Parallel::Parallel(Scheduler& scheduler,
                   const Composite::BehaviorList& children) :
    Composite(scheduler, children),
    mSuccessCount(0)
{
    ;
}

void Parallel::run()
{
    if(getChildren().empty())
    {
        notifySuccess();
        return;
    }

    // Schedule all the child behaviors at once.
    // Reversed to keep the execution sequence left-to-right using our sequential scheduler.
    for(std::vector<Behavior*>::const_reverse_iterator it = getChildren().rbegin();
        it != getChildren().rend(); ++it)
    {
        mScheduler.enqueue(*it);
    }

    setStatus(StatusWaiting);
}

void Parallel::terminate()
{
    if(getStatus() == StatusWaiting)
    {
        // Unschedule all the child behaviors at once.
        for(std::vector<Behavior*>::const_iterator it = getChildren().begin();
            it != getChildren().end(); ++it)
        {
            (*it)->terminate();
            mScheduler.dequeue(*it);
        }
    }
}

void Parallel::onSuccess(Behavior* behavior)
{
    UNUSED(behavior);

    if(++mSuccessCount == getChildren().size())
    {
        // Signal successful execution only after all children have succeeded.
        notifySuccess();
    }
}

void Parallel::onFailure(Behavior* behavior)
{
    // End all other remaining parallel tasks.
    for(std::vector<Behavior*>::const_iterator it = getChildren().begin();
        it != getChildren().end(); ++it)
    {
        if(*it != behavior)
        {
            (*it)->terminate();
            mScheduler.dequeue(*it);
        }
    }

    // Signal the failure of this node to the parent node.
    notifyFailure();
}

Decorator::Decorator(Scheduler& scheduler, Behavior* child) :
    mScheduler(scheduler),
    mChild(child)
{
    UNUSED(mScheduler);

    assert(mChild);
    mChild->setListener(this);
}

END_NS_AILIB
