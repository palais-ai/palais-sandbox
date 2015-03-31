#include "ScriptTimer.h"
#include "JavascriptBindings.h"
#include <qDebug>

qint32 ScriptTimer::sHandleCounter = 0;
QHash<qint32, ScriptTimer*> ScriptTimer::sScriptTimers;

qint32 ScriptTimer::newTimer(int interval,
                             bool oneShot,
                             QScriptEngine& engine,
                             const QScriptValue& function)
{
    if(function.isFunction())
    {
        ScriptTimer* timer = new ScriptTimer(interval, oneShot, engine, function);
        sScriptTimers.insert(timer->getHandle(), timer);
        return  timer->getHandle();
    }
    else
    {
        qWarning("ScriptTimer.newTimer: The second parameter to setTimeout/setInterval must be a function.");
        return -1;
    }
}

bool ScriptTimer::removeTimer(qint32 handle)
{
    QHash<qint32, ScriptTimer*>::iterator it = sScriptTimers.find(handle);

    if(it != sScriptTimers.end())
    {
        delete it.value();
        sScriptTimers.remove(handle);
        return true;
    }
    else
    {
        return false;
    }
}

void ScriptTimer::updateAll(float deltaTime)
{
    foreach(ScriptTimer* timer, ScriptTimer::sScriptTimers)
    {
        timer->update(deltaTime);
    }
}

void ScriptTimer::update(float deltaTime)
{
    mTimeLeft -= deltaTime;

    if(mTimeLeft < 0)
    {
        timeout();

        if(mIsOneShot)
        {
            if(sScriptTimers.remove(mHandle) != 1)
            {
                qWarning("Failed to remove single shot timer with id %d.", mHandle);
            }
        }
        else
        {
            mTimeLeft = mInitialTime;
        }
    }
}


qint32 ScriptTimer::getHandle() const
{
    return mHandle;
}

QScriptEngine& ScriptTimer::getEngine()
{
    return mEngine;
}

void ScriptTimer::onEngineDestroyed(QObject* engine)
{
    int removedCount = 0;

    QMutableHashIterator<qint32, ScriptTimer*> i(sScriptTimers);
    while (i.hasNext())
    {
        QHash<qint32, ScriptTimer*>::iterator it = i.next();
        ScriptTimer* timer = it.value();

        // Remove all timers registered with that engine.
        if (&timer->getEngine() == engine)
        {
            delete timer;
            i.remove();
            removedCount++;
        }
    }

    qDebug() << "Removed " << removedCount << " timers on script destruction.";
}

ScriptTimer::ScriptTimer(int interval,
                         bool oneShot,
                         QScriptEngine& engine,
                         const QScriptValue& function) :
    mTimeLeft(interval / 1000.f),
    mInitialTime(mTimeLeft),
    mIsOneShot(oneShot),
    mEngine(engine),
    mFunction(function),
    mHandle(ScriptTimer::sHandleCounter++)
{
    ;
}

void ScriptTimer::timeout()
{
    if(mFunction.isFunction())
    {
        mFunction.call();
    }
    else
    {
        qWarning("ScriptTimer.timeout: The second parameter to setTimeout/setInterval must be a function.");
    }

    JavaScriptBindings::checkScriptEngineException(mEngine, "script timer's timeout");
}

