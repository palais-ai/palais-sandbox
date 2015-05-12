#include "Behavior.h"
#include "Scene.h"
#include "Actor.h"
#include "BehaviorPrototypes.h"
#include <QDebug>

Q_DECLARE_METATYPE(ailib::Behavior*)
Q_DECLARE_METATYPE(Scheduler*)

BlackboardDecorator::BlackboardDecorator(Scheduler& scheduler,
                                         Behavior* child,
                                         Actor* actor,
                                         QString observedValue) :
    Decorator(scheduler, child),
    mActor(actor),
    mObservedValue(observedValue)
{
    AI_ASSERT(actor, "Actor mustn't be NULL.");
}

BlackboardDecorator::~BlackboardDecorator()
{
    ;
}

void BlackboardDecorator::run()
{
    if(getStatus() == StatusRunning)
    {
        connect(mActor, &KnowledgeModel::knowledgeChanged,
                this, &BlackboardDecorator::onKnowledgeChanged);
    }

    QVariantMap& state = *any_cast<QVariantMap*>(getUserData());
    QVariantMap::iterator it = state.find(mObservedValue);
    if(it != state.end() && it.value().toBool())
    {
        scheduleBehavior();

        if(getStatus() != StatusRunning)
        {
            notifyReset();
        }

        setStatus(StatusWaiting);
    }
    else
    {
        notifyFailure();
    }
}

void BlackboardDecorator::terminate()
{
    disconnect(mActor, &KnowledgeModel::knowledgeChanged,
               this, &BlackboardDecorator::onKnowledgeChanged);
    Decorator::terminate();
}

void BlackboardDecorator::onKnowledgeChanged(const QString& key, const QVariant& knowledge)
{
    UNUSED(knowledge);
    if(key == mObservedValue)
    {
        if(getStatus() == StatusWaiting)
        {
            terminateChild();
        }

        run();
    }
}

void behavior_tree_register_prototypes(QScriptEngine& engine)
{
    Status_register(engine);
    Behavior_register_prototype(engine);
}

void Status_register(QScriptEngine& engine)
{
    QScriptValue obj = engine.newObject();

    QScriptValue::PropertyFlags flags = QScriptValue::Undeletable | QScriptValue::ReadOnly;
    obj.setProperty("Waiting",
                    engine.toScriptValue((uint32_t)StatusDormant),
                    flags);
    obj.setProperty("Running",
                    engine.toScriptValue((uint32_t)StatusRunning),
                    flags);
    obj.setProperty("Waiting",
                    engine.toScriptValue((uint32_t)StatusWaiting),
                    flags);
    obj.setProperty("Terminated",
                    engine.toScriptValue((uint32_t)StatusTerminated),
                    flags);

    engine.globalObject().setProperty("Status",
                                      obj,
                                      flags);
}

void Behavior_register_prototype(QScriptEngine& engine)
{
    const int typeId = qRegisterMetaType<Behavior*>("Behavior*");

    static BehaviorPrototype bp;
    QScriptValue prototype = engine.newQObject(&bp);
    engine.setDefaultPrototype(typeId, prototype);

    QMap<QString, QScriptEngine::FunctionSignature> ctors;
    ctors["Behavior"] = Behavior_prototype_ctor;
    ctors["Sequence"] = composite_prototype_ctor<Sequence>;
    ctors["Selector"] = composite_prototype_ctor<Selector>;
    ctors["Parallel"] = composite_prototype_ctor<Parallel>;
    ctors["RandomSequence"] = composite_prototype_ctor<RandomSequence>;
    ctors["RandomSelector"] = composite_prototype_ctor<RandomSelector>;
    ctors["BlackboardDecorator"] = BlackboardDecorator_prototype_ctor;

    QScriptValue::PropertyFlags flags = QScriptValue::Undeletable | QScriptValue::ReadOnly;
    for(QMap<QString, QScriptEngine::FunctionSignature>::iterator it = ctors.begin();
        it != ctors.end(); ++it)
    {
        QScriptValue ctor = engine.newFunction(it.value());
        ctor.setProperty("prototype", prototype);
        engine.globalObject().setProperty(it.key(), ctor, flags);
    }
}

QScriptValue Behavior_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    return construct_shared_behavior(context, engine, new ScriptBehavior(context->thisObject()));
}

QScriptValue construct_shared_behavior(QScriptContext* context,
                                       QScriptEngine* engine,
                                       Behavior* ptr)
{
    context->thisObject().setProperty("__behavior",
                                      engine->toScriptValue(ptr),
                                      QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return engine->undefinedValue();
}

QScriptValue BlackboardDecorator_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    static const int numArgsExpected = 3;

    Behavior* child = NULL;
    Actor* actor = NULL;
    QString observedName;
    if(context->argumentCount() >= numArgsExpected)
    {
        {
            QScriptValue originalArg = context->argument(0);
            QScriptValue arg = originalArg.property("__behavior");
            Behavior* ptr = qscriptvalue_cast<Behavior*>(arg);

            if(!ptr)
            {
                QString msg("BlackboardDecorator.prototype.ctor: Argument 1 must be a behavior.");
                return context->throwError(QScriptContext::TypeError, msg);
            }
            child = ptr;
        }

        {
            QScriptValue arg = context->argument(1);
            actor = qscriptvalue_cast<Actor*>(arg);
            if(!actor)
            {
                QString msg("BlackboardDecorator.prototype.ctor: Argument 2 must be an Actor.");
                return context->throwError(QScriptContext::TypeError, msg);
            }
        }

        {
            QScriptValue arg = context->argument(2);
            if(!arg.isString())
            {
                QString msg("BlackboardDecorator.prototype.ctor: Argument 3 must be a string.");
                return context->throwError(QScriptContext::TypeError, msg);
            }
            observedName = arg.toString();
        }
    }
    else
    {
        QString msg;
        msg = QString("BlackboardDecorator.prototype.ctor: You must supply at least %1 arguments.")
                      .arg(numArgsExpected);
        return context->throwError(QScriptContext::TypeError,
                                   msg);
    }

    Scheduler* scheduler = qscriptvalue_cast<Scheduler*>(engine->globalObject()
                                                               .property("Scheduler"));
    AI_ASSERT(scheduler, "A Scheduler must exist in the global object.");

    construct_shared_behavior(context,
                              engine,
                              new BlackboardDecorator(*scheduler,
                                                      child,
                                                      actor,
                                                      observedName));

    QScriptValue array = engine->newArray();
    array.property("push").call(array, QScriptValueList() << context->argument(0));

    // Increase the internal ref count of the child node.
    context->thisObject().setProperty("children",
                                      array,
                                      QScriptValue::Undeletable | QScriptValue::ReadOnly);

    return engine->undefinedValue();
}
