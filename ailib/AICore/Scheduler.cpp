#include "Scheduler.h"

#include "HighResolutionTime.h"
#include <algorithm>
#include <queue>
#include <iostream>
#include <typeinfo>

BEGIN_NS_AILIB

#define PRINT_STATES 0

Scheduler::Scheduler() :
    mListener(NULL)
{
    ;
}

void Scheduler::setListener(SchedulerListener* listener)
{
    mListener = listener;
}

void Scheduler::clear()
{
    std::cout << "Cleared " << mTasks.size() + mWaiting.size() << " in Scheduler.";
    while(!mTasks.empty())
    {
        Task* current = *mTasks.begin();
        current->setStatus(StatusTerminated);
    }
    while(!mWaiting.empty())
    {
        Task* current = *mWaiting.begin();
        current->setStatus(StatusTerminated);
    }
}

void Scheduler::enqueue(Task* task)
{
    AI_ASSERT(task, "Enqueued tasks may not be NULL.");

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
        task->resetRuntime();
        mTasks.insert(task);
    }
    task->setListener(this);

    if(mListener)
    {
        mListener->onTaskAdded(task);
    }
}

void Scheduler::dequeue(Task* task)
{
    AI_ASSERT(task, "Dequeued tasks may not be NULL.");
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
        AI_ASSERT(false, "Only waiting or running tasks may be removed.");
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

HighResolutionTime::Timestamp Scheduler::update(HighResolutionTime::Timestamp maxRuntime, float dt)
{
    UNUSED(dt);
    using namespace HighResolutionTime;

    Timestamp currentRuntime = 0;
    while(!mTasks.empty() && currentRuntime <= maxRuntime)
    {
        Timestamp start = now();

        // Take tasks from the end (lowest runtime to date).
        Task* current = *mTasks.begin();

        if(mListener)
        {
            mListener->onBeginRunTask(current);
        }

        // Ignore changes to the status of this task during its execution time.
        current->setListener(NULL);
#if PRINT_STATES
        std::cout << "Running " << typeid(*current).name() << "." << std::endl;
#endif

        AI_ASSERT(current->getStatus() == StatusRunning,
                  "All tasks in the task queue must be running.");

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

        if(current->getStatus() != StatusTerminated)
        {
            current->setListener(this);
        }

#if PRINT_STATES
        printTasks(mTasks);
        printTasks(mWaiting);
#endif
    }

    return currentRuntime;
}

void Scheduler::onStatusChanged(Task* task, Status from)
{
    // FIXME: Some tasks get lost in changes (probably from changing to running / waiting when they are not being executed..
    //        Allow this to happen by checking for the currently executed task here explicitly.
    if(from == StatusWaiting)
    {
        removeWaiting(task);
    }
    else if(from == StatusRunning)
    {
        removeRunning(task);
    }

    const Status to = task->getStatus();
    if(to == StatusRunning || to == StatusWaiting)
    {
        enqueue(task);
    }
    else if(to == StatusTerminated)
    {
        task->setListener(NULL);
    }
}

void Scheduler::removeWaiting(Task* task)
{
    removeFrom(mWaiting, task);
}

void Scheduler::removeRunning(Task* task)
{
    removeFrom(mTasks, task);
}

void Scheduler::removeFrom(TaskList& list, Task* task)
{
    if(list.erase(task) != 1)
    {
        puts("Couldn't find task to erase.");
    }
    task->setListener(NULL);

    if(mListener)
    {
        mListener->onTaskRemoved(task);
    }
}

END_NS_AILIB
