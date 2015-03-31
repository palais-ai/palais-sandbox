#include "ScriptTimer.h"
#include "JavascriptBindings.h"
#include <qDebug>

ScriptTimerFactory::ScriptTimerFactory(QObject* parent) :
    QObject(parent),
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
        mScriptTimers.insert(mHandleCounter, timer);
        return mHandleCounter++;
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
        mScriptTimers.remove(handle);
        // DeleteLater is necessary here, because a timer may be removed while it is running.
        timer->deleteLater();
        return true;
    }
    else
    {
        return false;
    }
}

void ScriptTimerFactory::updateTimers(float deltaTime)
{
    QMutableHashIterator<qint32, ScriptTimer*> it(mScriptTimers);
    while(it.hasNext())
    {
        ScriptTimer* timer  = it.next().value();
        if(timer->update(deltaTime))
        {
            it.remove();
            delete timer;
        }
    }
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

