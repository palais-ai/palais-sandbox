#include "TimedLogger.h"
#include <QDebug>

TimedLogger::TimedLogger() :
    mStart(QTime::currentTime())
{
}

void TimedLogger::start()
{
    mStart = QTime::currentTime();
}

void TimedLogger::stop(const QString& activityName)
{
    qDebug() << QString("[ %0 ] took %1 seconds.")
                        .arg(activityName)
                        .arg(mStart.msecsTo(QTime::currentTime()) / 1000.f);
}
