#include "BehaviorPrototypes.h"
#include <QScriptEngine>
#include <QDebug>
#include "Bindings/JavascriptBindings.h"
#include "Actor.h"

using namespace ailib;

Q_DECLARE_METATYPE(ailib::Behavior*)
Q_DECLARE_METATYPE(Scheduler*)
Q_DECLARE_METATYPE(QVariantMap*)

ScriptBehavior::ScriptBehavior(const QScriptValue& obj) :
    mScript(obj)
{
    ;
}

ScriptBehavior::~ScriptBehavior()
{
    ;
}

void ScriptBehavior::run()
{
    QScriptValue runVal = mScript.property("run");
    if(runVal.isFunction())
    {
        runVal.call(mScript);
        JavaScriptBindings::checkScriptEngineException(*runVal.engine(),
                                                       "ScriptBehavior.run");
    }
    else
    {
        qWarning("ScriptBehavior.run: ScriptBehaviors should overwrite the __run__ method"
                 "-- Notifying failure.");
        notifyFailure();
    }
}

void ScriptBehavior::terminate()
{
    QScriptValue terminateVal = mScript.property("terminate");
    if(terminateVal.isFunction())
    {
        terminateVal.call(mScript);
        JavaScriptBindings::checkScriptEngineException(*terminateVal.engine(),
                                                       "ScriptBehavior.terminate");
    }
    else
    {
        // This is not an error case. Overriding terminate is optional.
    }

    Behavior::terminate();
}

static Behavior* extractBehavior(const QScriptValue& value)
{
    QScriptValue prop = value.property("__behavior");

    if(!prop.isValid())
    {
        qWarning("BehaviorPrototypes.extractBehavior: A ScriptBehavior didn't have its internal data set. "
                 "Did you forget to call the 'Behavior' constructor?");
        return NULL;
    }

    Behavior* ptr = qscriptvalue_cast<Behavior*>(prop);

    if(!ptr)
    {
        qWarning("BehaviorPrototypes.extractBehavior: Accessing deleted behavior.");
    }

    return ptr;
}

BehaviorPrototype::BehaviorPrototype(QObject* parent) :
    QObject(parent)
{
    ;
}

void BehaviorPrototype::setUserData(QVariantMap* data)
{
    Behavior* behavior = extractBehavior(thisObject());
    if(!behavior)
    {
        return;
    }

    behavior->setUserData(hold_any(data));
}

QVariantMap BehaviorPrototype::getUserData() const
{
    Behavior* behavior = extractBehavior(thisObject());
    if(!behavior || behavior->getUserData().empty())
    {
        return QVariantMap();
    }

    QVariantMap* ptr = any_cast<QVariantMap*>(behavior->getUserData());
    if(!ptr)
    {
        qWarning("Behavior.userData: Failed to convert userData to Object. Type mismatch.");
        return QVariantMap();
    }
    else
    {
        return *ptr;
    }
}

void BehaviorPrototype::setStatus(int status)
{
    if(status >= StatusDormant && status <= StatusTerminated)
    {
        setStatus((Status)status);
    }
    else
    {
        qWarning("Behavior.setStatus: Called with invalid status value.");
    }
}

void BehaviorPrototype::setStatus(Status status)
{
    Behavior* behavior = extractBehavior(thisObject());
    if(!behavior)
    {
        return;
    }
    behavior->setStatus(status);
}

Status BehaviorPrototype::getStatus() const
{
    Behavior* behavior = extractBehavior(thisObject());
    if(!behavior)
    {
        return StatusDormant;
    }
    return behavior->getStatus();
}

void BehaviorPrototype::notifySuccess()
{
    Behavior* behavior = extractBehavior(thisObject());
    if(!behavior)
    {
        return;
    }
    behavior->notifySuccess();
}

void BehaviorPrototype::notifyFailure()
{
    Behavior* behavior = extractBehavior(thisObject());
    if(!behavior)
    {
        return;
    }
    behavior->notifyFailure();
}

void BehaviorPrototype::notifyReset()
{
    Behavior* behavior = extractBehavior(thisObject());
    if(!behavior)
    {
        return;
    }
    behavior->notifyReset();
}

SchedulerPrototype::SchedulerPrototype(QObject* parent) :
    QObject(parent)
{
    ;
}

void SchedulerPrototype::enqueue(QScriptValue behaviorValue)
{
    if(mActiveBehaviors.contains(behaviorValue.objectId()))
    {
        qWarning("Scheduler.enqueue: Can't schedule the same behavior twice.");
        return;
    }

    // Prevent garbage collection of the executed behavior by keeping a reference.
    mActiveBehaviors[behaviorValue.objectId()] = behaviorValue;

    Scheduler* sched = qscriptvalue_cast<Scheduler*>(thisObject());
    Behavior* behavior = extractBehavior(behaviorValue);
    mAddedBehaviors[behaviorValue.objectId()] = behavior;

    if(!behavior)
    {
        qWarning("Scheduler.enqueue: Behavior pointer is missing from a ScriptBehavior. "
                 "Can't enqueue.");
        return;
    }
    sched->enqueue(behavior);
}

static void removeTerminatedRecursive(QScriptValue behaviorValue)
{
    QScriptValue children = behaviorValue.property("children");

    if(children.isArray())
    {
        const int len = children.property("length").toInt32();
        for(int i = 0; i < len; ++i)
        {
            QScriptValue child = children.property(i);
            removeTerminatedRecursive(child);
        }
    }

    Behavior* behavior = extractBehavior(behaviorValue);
    if(behavior)
    {
        assert(behavior->getStatus() == StatusTerminated ||
               behavior->getStatus() == StatusDormant);

        // TODO: This delete leads to crashes due to heap corruption,
        //       leak the memory until we discover the bug.
        //delete behavior;

        behaviorValue.setProperty("__behavior", QScriptValue());
    }
    else
    {
        AI_ASSERT(false, "Internal behavior pointer must be set.");
    }
}

void SchedulerPrototype::dequeue(QScriptValue behaviorValue)
{
    if(!mActiveBehaviors.contains(behaviorValue.objectId()))
    {
        qWarning("Scheduler.dequeue: Can't dequeue behavior that hasn't been enqueued before.");
        return;
    }

    Behavior* behavior = extractBehavior(behaviorValue);
    assert(behavior == mAddedBehaviors[behaviorValue.objectId()]);

    if(!behavior)
    {
        qWarning("Scheduler.dequeue: Behavior pointer is missing from a ScriptBehavior. "
                 "Can't dequeue.");
        return;
    }

    // Cascade terminate the behaviors from the root.
    // This removes all references from the scheduler.
    behavior->terminate();

    // Destroy this behavior and its children.
    removeTerminatedRecursive(behaviorValue);

    // Remove reference so that the behavior can be garbage collected by the script engine.
    int numRemoved = mActiveBehaviors.remove(behaviorValue.objectId());
    assert(numRemoved == 1);
}
