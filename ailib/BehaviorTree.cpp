#include "BehaviorTree.h"

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

Composite::Composite(const Composite::ChildList& children) :
    mChildren(children)
{
    for(Composite::ChildList::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        (*it)->setListener(this);
    }
}

const Composite::ChildList& Composite::getChildren() const
{
    return mChildren;
}

Selector::Selector(const Composite::ChildList& children) :
    Composite(children)
{
    ;
}

void Selector::run()
{
    for(Composite::ChildList::const_iterator it = getChildren().begin();
        it != getChildren().end(); ++it)
    {
        (*it)->run();

        if(shouldYield())
        {
            return;
        }
    }

    notifyFailure();
}

void Selector::onSuccess(Behavior* behavior)
{
    notifySuccess();
}

void Selector::onFailure(Behavior* behavior)
{
    // Do nothing.
}

Sequence::Sequence(const Composite::ChildList& children) :
    Composite(children)
{
    ;
}

void Sequence::run()
{
    for(Composite::ChildList::const_iterator it = getChildren().begin();
        it != getChildren().end(); ++it)
    {
        (*it)->run();

        if(shouldYield())
        {
            return;
        }
    }

    notifySuccess();
}

void Sequence::onSuccess(Behavior* behavior)
{
    UNUSED(behavior);
    // Do nothing.
}

void Sequence::onFailure(Behavior* behavior)
{
    UNUSED(behavior);
    notifyFailure();
}

Parallel::Parallel(Scheduler& scheduler, const Composite::ChildList& children) :
    Composite(children),
    mScheduler(scheduler),
    mSuccessCount(0)
{
    ;
}

void Parallel::run()
{
    for(std::vector<Behavior*>::const_iterator it = getChildren().begin();
        it != getChildren().end(); ++it)
    {
        enqueue(*it);
    }
}

void Parallel::onSuccess(Behavior* behavior)
{
    UNUSED(behavior);

    if(++mSuccessCount == getChildren().size())
    {
        notifySuccess();
    }
}

void Parallel::onFailure(Behavior* behavior)
{
    for(std::vector<Behavior*>::const_iterator it = getChildren().begin();
        it != getChildren().end(); ++it)
    {
        if(*it != behavior)
        {
            dequeue(*it);
        }
    }

    notifyFailure();
}

void Parallel::enqueue(Behavior* behavior)
{
    mScheduler.enqueue(behavior);
}

void Parallel::dequeue(Behavior* behavior)
{
    mScheduler.dequeue(behavior);
}

END_NS_AILIB
