#include "Behavior.h"
#include "scene.h"
#include "actor.h"

#include "BehaviorPrototypes.h"

Q_DECLARE_METATYPE(QSharedPointer<Behavior>)
Q_DECLARE_METATYPE(QSharedPointer<Behavior>*)
Q_DECLARE_METATYPE(Scheduler*)

BlackboardDecorator::BlackboardDecorator(Scheduler& scheduler,
                                         Behavior* child,
                                         QString observedValue) :
    Decorator(scheduler, child),
    mObservedValue(observedValue),
    mListenerHandle(0)
{
    ;
}

void BlackboardDecorator::onValueChanged(const btHashString& key, const ailib::hold_any& value)
{
    UNUSED(value);

    if(getStatus() == StatusWaiting)
    {
        terminateChild();
    }

    if(key.equals(mObservedValue.toLocal8Bit().constData()))
    {
        run();
    }
}

void BlackboardDecorator::run()
{
    mListenerHandle = mBlackboard.addListener(this);

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

void BlackboardDecorator::terminate()
{
    mBlackboard.removeListener(mListenerHandle);
}

void behavior_tree_register_prototypes(QScriptEngine& engine)
{
    Behavior_register_prototype(engine);
}

void Behavior_register_prototype(QScriptEngine& engine)
{
    const int typeId = qRegisterMetaType<Behavior*>("Behavior*");

    BehaviorPrototype bp;
    engine.setDefaultPrototype(typeId, engine.newQObject(&bp));

    engine.globalObject().setProperty("Behavior",
                                      engine.newFunction(Behavior_prototype_ctor));
    engine.globalObject().setProperty("Sequence",
                                      engine.newFunction(composite_prototype_ctor<Sequence>));
    engine.globalObject().setProperty("Selector",
                                      engine.newFunction(composite_prototype_ctor<Selector>));
    engine.globalObject().setProperty("Parallel",
                                      engine.newFunction(composite_prototype_ctor<Parallel>));
    engine.globalObject().setProperty("BlackboardDecorator",
                                      engine.newFunction(BlackboardDecorator_prototype_ctor));
}

QScriptValue Behavior_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    return construct_shared_behavior(context, engine, new ScriptBehavior(context->thisObject()));
}

QScriptValue construct_shared_behavior(QScriptContext* context,
                                       QScriptEngine* engine,
                                       Behavior* ptr)
{
    context->thisObject().setData(engine->toScriptValue(QSharedPointer<Behavior>(ptr)));

    AI_ASSERT(qscriptvalue_cast<QSharedPointer<Behavior>* >(context->thisObject().data()),
              "Data wasnt set.");

    return engine->undefinedValue();
}

QScriptValue BlackboardDecorator_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    Behavior* child = NULL;
    QString observedName;
    if(context->argumentCount() >= 2)
    {
        {
            QScriptValue originalArg = context->argument(0);
            QScriptValue arg = originalArg.data();
            QSharedPointer<Behavior>* ptr = qscriptvalue_cast<QSharedPointer<Behavior>* >(arg);

            if(!ptr)
            {
                QString msg("BlackboardDecorator.prototype.ctor: Argument 1 must be a behavior.");
                return context->throwError(QScriptContext::TypeError, msg);
            }

            child = ptr->data();
        }

        {
            QScriptValue arg = context->argument(1);
            if(!arg.isString())
            {
                QString msg("BlackboardDecorator.prototype.ctor: Argument 2 must be a string.");
                return context->throwError(QScriptContext::TypeError, msg);
            }
            observedName = arg.toString();
        }
    }
    else
    {
        QString msg("BlackboardDecorator.prototype.ctor: You must supply at least 2 arguments.");
        return context->throwError(QScriptContext::TypeError,
                                   msg);
    }

    Scheduler* scheduler = qscriptvalue_cast<Scheduler*>(engine->globalObject()
                                                               .property("Scheduler"));
    AI_ASSERT(scheduler, "A Scheduler must be exist in the global object.");

    return construct_shared_behavior(context, engine, new BlackboardDecorator(*scheduler,
                                                                              child,
                                                                              observedName));
}
