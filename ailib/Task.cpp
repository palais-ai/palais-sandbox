#include "Task.h"

BEGIN_NS_AILIB

Task::Task() :
    mStatus(StatusUnknown)
{
    ;
}

Task::~Task()
{
    ;
}

void Task::interrupt()
{
    mStatus = StatusInterrupted;
}

bool Task::shouldYield() const
{
    return getStatus() != StatusRunning;
}

Task::Status Task::getStatus() const
{
    return mStatus;
}

void Task::setStatus(Task::Status status)
{
    mStatus = status;
}

END_NS_AILIB
