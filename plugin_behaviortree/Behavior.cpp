#include "Behavior.h"
#include "scene.h"
#include "actor.h"
#include <QScriptEngine>
#include <QDebug>
#include <QScriptEngine>
#include <QDebug>

Q_DECLARE_METATYPE(Sequence*)

using namespace ailib;

BlackboardDecorator::BlackboardDecorator(Scheduler& scheduler,
                                         Behavior* child,
                                         QString observedValue,
                                         const Blackboard<btHashString>& blackboard) :
    Decorator(scheduler, child),
    mObservedValue(observedValue),
    mBlackboard(blackboard)
{
    ;
}

void BlackboardDecorator::onValueChanged(const btHashString& key, const ailib::hold_any& value)
{
    UNUSED(value);
    if(key.equals(mObservedValue.toLocal8Bit().constData()))
    {
        setStatus(StatusRunning);
    }
}

void BlackboardDecorator::run()
{
    if(mBlackboard.get<bool>(mObservedValue.toLocal8Bit().constData()))
    {
        scheduleBehavior();
        setStatus(StatusWaiting);
    }
    else
    {
        notifyFailure();
    }
}

void Sequence_register_prototype(QScriptEngine& engine)
{
    ;
}

QScriptValue Sequence_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    ;
}
