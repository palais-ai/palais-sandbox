#ifndef TASK_H
#define TASK_H

#pragma once

#include "ai_global.h"
#include "HighResolutionTime.h"

BEGIN_NS_AILIB

class Task
{
public:
    enum Status
    {
        StatusUnknown = 0,
        StatusDone,
        StatusRunning,
        StatusWaiting
    };

    Task();
    virtual ~Task();
    virtual void run() = 0;

    Status getStatus() const;
    void setStatus(Status status);
    void addRuntime(const HighResolutionTime::Timestamp& runtime);
    HighResolutionTime::Timestamp getRuntime() const;
private:
    uint16_t mRuntime; // in microseconds
    Status mStatus;
};

END_NS_AILIB

#endif // TASK_H
