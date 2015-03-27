#include "BehaviorPrototypes.h"
#include <QScriptEngine>
#include <QDebug>
#include "javascriptbindings.h"

using namespace ailib;

Q_DECLARE_METATYPE(QSharedPointer<Behavior>)
Q_DECLARE_METATYPE(Scheduler*)

ScriptBehavior::ScriptBehavior(const QScriptValue& obj) :
    mScript(obj)
{
    ;
}

void ScriptBehavior::run()
{
    QScriptValue runVal = mScript.property("run");
    if(runVal.isFunction())
    {
        runVal.call();
        JavaScriptBindings::checkScriptEngineException(*runVal.engine(),
                                                       "ScriptBehavior.run");
    }
    else
    {
        qWarning("ScriptBehavior should overwrite the __run__ method -- Notifying failure.");
        notifyFailure();
    }
}

void ScriptBehavior::terminate()
{
    QScriptValue terminateVal = mScript.property("terminate");
    if(terminateVal.isFunction())
    {
        terminateVal.call();
        JavaScriptBindings::checkScriptEngineException(*terminateVal.engine(),
                                                       "ScriptBehavior.terminate");
    }
    else
    {
        // This is not an error case. Overriding terminate is optional.
    }

    Behavior::terminate();
}

FORCE_INLINE static Behavior* extractBehavior(const QScriptValue& value)
{
    return qscriptvalue_cast<QSharedPointer<Behavior> >(value.data()).data();
}

BehaviorPrototype::BehaviorPrototype(QObject* parent) :
    QObject(parent)
{
    ;
}

void BehaviorPrototype::setUserData(QVariantMap* data)
{
    Behavior* behavior = extractBehavior(thisObject());
    behavior->setUserData(hold_any(data));
}

const QVariantMap* BehaviorPrototype::getUserData() const
{
    Behavior* behavior = extractBehavior(thisObject());

    try
    {
        return any_cast<QVariantMap*>(behavior->getUserData());
    }
    catch(const bad_any_cast& ex)
    {
        UNUSED(ex);
        return NULL;
    }
}

void BehaviorPrototype::setStatus(Status status)
{
    Behavior* behavior = extractBehavior(thisObject());
    behavior->setStatus(status);
}

Status BehaviorPrototype::getStatus() const
{
    Behavior* behavior = extractBehavior(thisObject());
    return behavior->getStatus();
}

void BehaviorPrototype::notifySuccess()
{
    Behavior* behavior = extractBehavior(thisObject());
    behavior->notifySuccess();
}

void BehaviorPrototype::notifyFailure()
{
    Behavior* behavior = extractBehavior(thisObject());
    behavior->notifyFailure();
}

void BehaviorPrototype::notifyReset()
{
    Behavior* behavior = extractBehavior(thisObject());
    behavior->notifyReset();
}

SchedulerPrototype::SchedulerPrototype(QObject* parent) :
    QObject(parent)
{
    ;
}

void SchedulerPrototype::enqueue(QScriptValue behaviorValue)
{
    Scheduler* sched = qscriptvalue_cast<Scheduler*>(thisObject());
    Behavior* behavior = extractBehavior(behaviorValue);
    sched->enqueue(behavior);
}

void SchedulerPrototype::dequeue(QScriptValue behaviorValue)
{
    Scheduler* sched = qscriptvalue_cast<Scheduler*>(thisObject());
    Behavior* behavior = extractBehavior(behaviorValue);
    sched->dequeue(behavior);
}
