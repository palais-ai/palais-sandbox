#include "Scheduler.h"

#include "HighResolutionTime.h"
#include <algorithm>
#include <queue>
#include <cassert>
#include <iostream>
#include <typeinfo>

BEGIN_NS_AILIB

#define PRINT_STATES 0

void Scheduler::enqueue(Task* task)
{
    assert(task);

#if PRINT_STATES
    std::cout << "Adding " << typeid(*task).name() << "." << std::endl;
#endif

    if(task->getStatus() == StatusWaiting)
    {
        mWaiting.insert(task);
    }
    else
    {
        task->setStatus(StatusRunning);
        mTasks.insert(task);
    }
    task->setListener(this);
}

void Scheduler::dequeue(Task* task)
{
    assert(task);

#if PRINT_STATES
    std::cout << "Removing " << typeid(*task).name() << "." << std::endl;
#endif
    const Status status = task->getStatus();
    if(status == StatusWaiting)
    {
        removeWaiting(task);
    }
    else if(status == StatusRunning)
    {
        removeRunning(task);
    }
    else
    {
        puts("Only waiting or running tasks may be removed.");
    }
}

#if PRINT_STATES
static void printTasks(const Scheduler::TaskList& tasks)
{
    std::cout << "Vector(" << tasks.size() << ") [ ";
    for(Scheduler::TaskList::const_iterator it = tasks.begin(); it != tasks.end(); ++it)
    {
        std::cout << typeid(*(*it)).name() << ",";
    }
    std::cout << " ]" << std::endl;
}
#endif

void Scheduler::update(HighResolutionTime::Timestamp maxRuntime, float dt)
{
    UNUSED(dt);

    using namespace HighResolutionTime;

    Timestamp currentRuntime = 0;

    while(!mTasks.empty() && currentRuntime <= maxRuntime)
    {
        Timestamp start = now();

        // Take tasks from the end (lowest runtime to date).
        Task* current = *mTasks.begin();
#if PRINT_STATES
        std::cout << "Running " << typeid(*current).name() << "." << std::endl;
#endif

        switch(current->getStatus())
        {
            case StatusRunning:
            {
                dequeue(current);

                // Execute the current task.
                current->run();

                Timestamp duration = now() - start;
                currentRuntime += duration;

                // Add the granted computation time to the tasks runtime if it isn't done yet.
                if(current->getStatus() == StatusRunning ||
                   current->getStatus() == StatusWaiting)
                {
                    // Re-insert it at the appropiate position in the task queue
                    current->addRuntime(duration);
                    enqueue(current);
                }

#if PRINT_STATES
                printTasks(mTasks);
                printTasks(mWaiting);
#endif
            }
            break;
        case StatusWaiting:
            break;
        case StatusDormant:
        default:
            // Remove finished and unknwon status tasks.
            // The queue should only contain running and waiting tasks.
            dequeue(current);
            break;
        }
    }
}

void Scheduler::onStatusChanged(Task* task, Status from)
{
    const Status to = task->getStatus();
    if(to == StatusRunning &&
       from == StatusWaiting)
    {
        removeWaiting(task);
        enqueue(task);
    }
    else if(to == StatusWaiting &&
            from != StatusRunning)
    {
        enqueue(task);
    }
    else if(from == StatusWaiting)
    {
        removeWaiting(task);
    }
    else if(from == StatusRunning)
    {
        removeRunning(task);
    }
}

void Scheduler::removeWaiting(Task* task)
{
    if(mWaiting.erase(task) != 1)
    {
        printf("Couldnt find task to erase. %lu\n", mWaiting.size());
    }

    task->setListener(NULL);
}

void Scheduler::removeRunning(Task* task)
{
    if(mTasks.erase(task) != 1)
    {
        printf("Couldnt find task to erase.  %lu\n", mTasks.size());
    }

    task->setListener(NULL);
}

END_NS_AILIB
