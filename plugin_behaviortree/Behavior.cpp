#include "Behavior.h"
#include "scene.h"
#include "actor.h"

#include "BehaviorPrototypes.h"

Q_DECLARE_METATYPE(QSharedPointer<Behavior>)
Q_DECLARE_METATYPE(QSharedPointer<Behavior>*)
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

void BlackboardDecorator::run()
{
    connect(mActor, &KnowledgeModel::knowledgeChanged,
            this, &BlackboardDecorator::onKnowledgeChanged);

    QVariantMap& state = *any_cast<QVariantMap*>(getUserData());
    QVariantMap::iterator it = state.find(mObservedValue);
    if(it != state.end() && it.value().toBool())
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
    disconnect(mActor, &KnowledgeModel::knowledgeChanged,
               this, &BlackboardDecorator::onKnowledgeChanged);
}

void BlackboardDecorator::onKnowledgeChanged(const QString& key, const QVariant& knowledge)
{
    UNUSED(knowledge);
    if(getStatus() == StatusWaiting)
    {
        terminateChild();
    }

    if(key == mObservedValue)
    {
        run();
    }
}

void behavior_tree_register_prototypes(QScriptEngine& engine)
{
    Behavior_register_prototype(engine);
}

static void registerConstructorAndPrototype(QScriptEngine& engine,
                                            const QString& name,
                                            QScriptEngine::FunctionSignature constructorFun,
                                            const QScriptValue& prototype)
{
    QScriptValue ctor = prototype.engine()->newFunction(constructorFun);
    ctor.setPrototype(prototype);
    prototype.engine()->globalObject().setProperty(name, ctor);
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
    ctors["BlackboardDecorator"] = BlackboardDecorator_prototype_ctor;

    for(QMap<QString, QScriptEngine::FunctionSignature>::iterator it = ctors.begin();
        it != ctors.end(); ++it)
    {
        QScriptValue ctor = engine.newFunction(it.value());
        ctor.setProperty("prototype", prototype);
        engine.globalObject().setProperty(it.key(), ctor);
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
    context->thisObject().setData(engine->toScriptValue(QSharedPointer<Behavior>(ptr)));

    AI_ASSERT(qscriptvalue_cast<QSharedPointer<Behavior>* >(context->thisObject().data()),
              "Data wasnt set.");

    return engine->undefinedValue();
}

QScriptValue BlackboardDecorator_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    static const int numArgsExpected = 3;
    Behavior* child = NULL;
    Actor*    actor = NULL;
    QString observedName;
    if(context->argumentCount() >= numArgsExpected)
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

    return construct_shared_behavior(context, engine, new BlackboardDecorator(*scheduler,
                                                                              child,
                                                                              actor,
                                                                              observedName));
}
