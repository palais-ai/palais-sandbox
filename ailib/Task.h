#ifndef TASK_H
#define TASK_H

#pragma once

#include "ai_global.h"

BEGIN_NS_AILIB

class Task
{
public:
    enum Status
    {
        StatusUnknown = 0,
        StatusDone,
        StatusRunning,
        StatusInterrupted,
        StatusTerminated
    };

    Task();
    virtual ~Task();
    virtual void run() = 0;

    void interrupt();
    bool shouldYield() const;
    Status getStatus() const;
    void setStatus(Status status);
private:
    Status mStatus;
};

END_NS_AILIB

#endif // TASK_H
