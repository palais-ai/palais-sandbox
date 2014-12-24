#ifndef TIMEDLOGGER_H
#define TIMEDLOGGER_H

#include <QTime>

class TimedLogger
{
public:
    TimedLogger();

    void start();
    void stop(const QString& activityName);
private:
    QTime mStart;
};

#endif // TIMEDLOGGER_H
