#ifndef PLANNING_H
#define PLANNING_H

#include <QObject>
#include "BehaviorTree.h"
#include "Blackboard.h"

using namespace ailib;

class BehaviorTree : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE void setRoot(Behavior* behavior);
private:
    Behavior* mRoot;
};

class BlackboardDecorator : public Decorator, public BlackboardListener<btHashString>
{
public:
    BlackboardDecorator(Scheduler& scheduler,
                        Behavior* child,
                        QString observedValue,
                        Blackboard<btHashString>& blackboard) :
        Decorator(scheduler, child),
        mObservedValue(observedValue),
        mBlackboard(blackboard)
    {
        ;
    }

    virtual void onValueChanged(const btHashString& key, const ailib::hold_any& value)
    {
        if(key.equals(mObservedValue))
        {
            setStatus(StatusRunning);
        }
    }

    virtual void run()
    {
        if(mBlackboard.get<bool>(mObservedValue))
        {
            scheduleBehavior();
            setStatus(StatusWaiting);
        }
        else
        {
            notifyFailure();
        }
    }

private:
    QString mObservedValue;
    Blackboard<btHashString>& mBlackboard;
};



#endif // PLANNING_H
