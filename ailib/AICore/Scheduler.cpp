#include "Scheduler.h"

#include "HighResolutionTime.h"
#include <algorithm>
#include <queue>
#include <cassert>

BEGIN_NS_AILIB

class TaskComparator
{
public:
    FORCE_INLINE bool operator() (const Task* lv, const Task* rv) const
    {
        // Greather than ('>') because priority_queue orders from highest to lowest.
        // We require the opposite.
        return lv->getRuntime() > rv->getRuntime();
    }
};

void Scheduler::enqueue(Task* task)
{
    // Tasks that haven't been run yet are always inserted at the list's end.
    if(task->getRuntime() == 0)
    {
        // New tasks are automatically set to run.
        task->setStatus(StatusRunning);
        mTasks.push_back(task);
    }
    // Tasks with runtimes > 0 must be inserted at their sorted position in the queue.
    else
    {
        TaskComparator cmp;
        TaskList::iterator it = std::lower_bound(mTasks.begin(),
                                                 mTasks.end(),
                                                 task,
                                                 cmp);

        mTasks.insert(it, task);
    }
}

void Scheduler::dequeue(Task* task)
{
    if(task == mTasks.back())
    {
        mTasks.pop_back();
    }
    else
    {
        TaskComparator cmp;
        TaskList::iterator it = std::lower_bound(mTasks.begin(),
                                                 mTasks.end(),
                                                 task,
                                                 cmp);

        if(it != mTasks.end() && !(cmp(task, *it)))
        {
            assert(*it == task);
            mTasks.erase(it);
        }
        else
        {
            puts("Task couldnt be dequeued because its not currently queued.");
        }
    }
}

void Scheduler::run()
{
    using namespace HighResolutionTime;

    //const Timestamp maxRuntime = milliseconds(5);
    Timestamp currentRuntime = 0;

    while(!mTasks.empty() /* && currentRuntime <= maxRuntime */)
    {
        Timestamp start = now();

        // Take tasks from the end (lowest runtime to date).
        Task* current = mTasks.back();

        switch(current->getStatus())
        {
            case StatusRunning:
            {
                // Execute the current task.
                current->run();

                Timestamp duration = now() - start;
                currentRuntime += duration;

                // Add the granted computation time to the tasks runtime if it isn't done yet.
                if(current->getStatus() == StatusRunning)
                {
                    // Re-insert it at the appropiate position in the task queue
                    dequeue(current);
                    current->addRuntime(duration);
                    enqueue(current);
                }
            }
            break;
        case StatusWaiting:
            break;
        case StatusDone:
        case StatusUnknown:
        default:
            // Remove finished and unknwon status tasks.
            // The queue should only contain running and waiting tasks.
            dequeue(current);
            break;
        }
    }
}

END_NS_AILIB
