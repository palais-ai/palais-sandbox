#include "Scheduler.h"

#include <algorithm>

BEGIN_NS_AILIB

void Scheduler::enqueue(Task* task)
{
    task->setStatus(StatusRunning);
    mTasks.push_back(task);
}

void Scheduler::dequeue(Task* task)
{
    std::vector<Task*>::iterator it = std::find(mTasks.begin(), mTasks.end(), task);

    if(it != mTasks.end())
    {
        mTasks.erase(it);
    }
}

void Scheduler::run()
{
    int32_t idx = 0;
    while(!mTasks.empty())
    {
        Task* current = mTasks[idx];
        current->run();

        if(current->getStatus() != StatusRunning)
        {
            dequeue(current);
        }
        else
        {
            ++idx;
        }
    }
}

END_NS_AILIB
