#include "BehaviorPrototypes.h"
#include <QScriptEngine>
#include <QDebug>

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
        // FIXME: Add exception check.
        runVal.call();
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
        // FIXME: Add exception check.
        terminateVal.call();
    }
    else
    {
        // This is not an error case. Overriding terminate is optional.
    }

    Behavior::terminate();
}

BehaviorPrototype::BehaviorPrototype(QObject* parent) :
    QObject(parent)
{
    ;
}

QVariantMap BehaviorPrototype::getBlackboard() const
{
    return QVariantMap();
}

void BehaviorPrototype::setStatus(Status status)
{
    Behavior* behavior = qscriptvalue_cast<QSharedPointer<Behavior> >(thisObject().data()).data();
    behavior->setStatus(status);
}

void BehaviorPrototype::notifySuccess()
{
    Behavior* behavior = qscriptvalue_cast<QSharedPointer<Behavior> >(thisObject().data()).data();
    behavior->notifySuccess();
}

void BehaviorPrototype::notifyFailure()
{
    Behavior* behavior = qscriptvalue_cast<QSharedPointer<Behavior> >(thisObject().data()).data();
    behavior->notifyFailure();
}

void BehaviorPrototype::notifyReset()
{
    Behavior* behavior = qscriptvalue_cast<QSharedPointer<Behavior> >(thisObject().data()).data();
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
    Behavior* behavior = qscriptvalue_cast<QSharedPointer<Behavior> >(behaviorValue.data()).data();
    sched->enqueue(behavior);
}

void SchedulerPrototype::dequeue(QScriptValue behaviorValue)
{
    Scheduler* sched = qscriptvalue_cast<Scheduler*>(thisObject());
    Behavior* behavior = qscriptvalue_cast<QSharedPointer<Behavior> >(behaviorValue.data()).data();
    sched->dequeue(behavior);
}
