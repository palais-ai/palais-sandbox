#ifndef SCHEDULER_H
#define SCHEDULER_H

#pragma once

#include "ai_global.h"
#include "Task.h"
#include <set>

BEGIN_NS_AILIB

class TaskComparator
{
public:
    FORCE_INLINE bool operator() (const Task* lv, const Task* rv) const
    {
        if(lv->getRuntime() == rv->getRuntime())
        {
            return lv > rv;
        }

        return lv->getRuntime() < rv->getRuntime();
    }
};

class Scheduler : private TaskListener
{
public:
    typedef std::set<Task*, TaskComparator> TaskList;

    void clear();
    void enqueue(Task* task);
    void dequeue(Task* task);
    void update(HighResolutionTime::Timestamp maxRuntime, float dt);
    virtual void onStatusChanged(Task* task, Status from);
private:
    void removeWaiting(Task* task);
    void removeRunning(Task* task);

    TaskList mTasks;
    TaskList mWaiting;
};

END_NS_AILIB

#endif // SCHEDULER_H
