#ifndef SCHEDULER_H
#define SCHEDULER_H

#pragma once

#include "ai_global.h"
#include "Task.h"
#include <vector>

BEGIN_NS_AILIB

class Scheduler : public Task
{
public:
    typedef std::vector<Task*> TaskList;

    void enqueue(Task* task);
    void dequeue(Task* task);
    virtual void run();
private:
    TaskList mTasks;
};

END_NS_AILIB

#endif // SCHEDULER_H
