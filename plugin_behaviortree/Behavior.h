#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <QObject>
#include <QScriptValue>
#include <QVector>
#include <QScriptEngine>
#include <QDebug>
#include "BehaviorTree.h"
#include "Blackboard.h"

using namespace ailib;

class Actor;

class BlackboardDecorator : public QObject, public Decorator
{
    Q_OBJECT
public:
    BlackboardDecorator(Scheduler& scheduler,
                        Behavior* child,
                        Actor* actor,
                        QString observedValue);

    virtual void run();
    virtual void terminate();
public slots:
    void onKnowledgeChanged(const QString& key, const QVariant& knowledge);
private:
    Actor* mActor;
    QString mObservedValue;
};

void behavior_tree_register_prototypes(QScriptEngine& engine);
void Status_register(QScriptEngine& engine);
void Behavior_register_prototype(QScriptEngine& engine);

QScriptValue Behavior_prototype_ctor(QScriptContext *context, QScriptEngine *engine);
QScriptValue construct_shared_behavior(QScriptContext *context,
                                       QScriptEngine *engine,
                                       Behavior* ptr);

template <typename T>
QScriptValue composite_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    Composite::BehaviorList children;
    QScriptValueList sharedBehaviors;
    for(int i = 0; i < context->argumentCount(); ++i)
    {
        QScriptValue arg = context->argument(i);
        Behavior* behavior =
                qscriptvalue_cast<Behavior* >(arg.property("__behavior"));

        if(!behavior)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Composite.prototype.ctor: Arguments must be behaviors. Did you forget to call the Behaviors constructor for one of your user-defined behaviors?");
        }
        sharedBehaviors.append(arg);
        children.push_back(behavior);
    }
    Scheduler* scheduler = qscriptvalue_cast<Scheduler*>(engine->globalObject()
                                                               .property("Scheduler"));
    AI_ASSERT(scheduler, "A Scheduler must exist in the global object.");

    construct_shared_behavior(context, engine, new T(*scheduler, children));

    QScriptValue array = engine->newArray();
    foreach(QScriptValue behavior, sharedBehaviors)
    {
        array.property("push").call(array, QScriptValueList() << behavior);
    }

    // We need to keep a reference within the scripting system
    // to prevent garbage collection.
    context->thisObject().setProperty("children", array);
    return engine->undefinedValue();
}

QScriptValue BlackboardDecorator_prototype_ctor(QScriptContext *context, QScriptEngine *engine);

#endif // BEHAVIOR_H
