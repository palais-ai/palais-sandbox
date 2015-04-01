#include "ScriptTimer.h"
#include "JavascriptBindings.h"
#include <qDebug>
#include <cassert>

ScriptTimerFactory::ScriptTimerFactory(QObject* parent) :
    QObject(parent),
    mCurrentTimer(NULL),
    mCurrentIsRemoved(false),
    mHandleCounter(0)
{

}

qint32 ScriptTimerFactory::newTimer(int interval,
                                    bool oneShot,
                                    const QScriptValue& function)
{
    if(function.isFunction())
    {
        ScriptTimer* timer = new ScriptTimer(interval, oneShot, function);
        qint32 nextId = mHandleCounter;
        mHandleCounter++;
        mScriptTimers.insert(nextId, timer);
        return nextId;
    }
    else
    {
        qWarning("ScriptTimer.newTimer: The second parameter to setTimeout/setInterval must be a function.");
        return -1;
    }
}

bool ScriptTimerFactory::removeTimer(qint32 handle)
{
    QHash<qint32, ScriptTimer*>::iterator it = mScriptTimers.find(handle);
    if(it != mScriptTimers.end())
    {
        ScriptTimer* timer = it.value();
        if(timer != mCurrentTimer)
        {
            int numRemoved = mScriptTimers.remove(handle);
            assert(numRemoved == 1);
            delete timer;
        }
        else
        {
            // Delay deletion of the currently running timer until it is done.
            mCurrentIsRemoved = true;
        }
        return true;
    }
    else
    {
        return false;
    }
}

void ScriptTimerFactory::updateTimers(float deltaTime)
{
    // We can't use an ordinary iterator here,
    // because removeTimer could be called by timeout functions.
    QList<qint32> keys = mScriptTimers.keys();
    foreach(qint32 key, keys)
    {
        // Check on every iteration whether this key was removed by a previous timer.
        QHash<qint32, ScriptTimer*>::iterator it = mScriptTimers.find(key);
        if(it != mScriptTimers.end())
        {
            mCurrentTimer = it.value();
            if(mCurrentTimer->update(deltaTime) ||
               mCurrentIsRemoved)
            {
                mScriptTimers.remove(key);
                delete mCurrentTimer;
                mCurrentIsRemoved = false;
            }
        }
    }
    mCurrentTimer = NULL;
}

void ScriptTimerFactory::onEngineDestroyed(QObject* engine)
{
    int removedCount = 0;

    QMutableHashIterator<qint32, ScriptTimer*> i(mScriptTimers);
    while (i.hasNext())
    {
        ScriptTimer* timer = i.next().value();

        // Remove all timers registered with that engine.
        if (timer->getEngine() == engine)
        {
            delete timer;
            i.remove();
            removedCount++;
        }
    }

    qDebug() << "Removed " << removedCount << " timers on script destruction.";
}


bool ScriptTimer::update(float deltaTime)
{
    mTimeLeft -= deltaTime;

    if(mTimeLeft < 0)
    {
        timeout();

        if(mIsOneShot)
        {
            return true;
        }
        else
        {
            mTimeLeft = mInitialTime;
        }
    }
    return false;
}

QScriptEngine* ScriptTimer::getEngine()
{
    return mFunction.engine();
}

ScriptTimer::ScriptTimer(int interval,
                         bool oneShot,
                         const QScriptValue& function) :
    mTimeLeft(interval / 1000.f),
    mInitialTime(mTimeLeft),
    mIsOneShot(oneShot),
    mFunction(function)
{
    ;
}

void ScriptTimer::timeout()
{
    mFunction.call();
    JavaScriptBindings::checkScriptEngineException(*getEngine(), "script timer's timeout");
}

