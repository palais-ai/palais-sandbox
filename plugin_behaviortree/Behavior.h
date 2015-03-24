#ifndef PLANNING_H
#define PLANNING_H

#include <QObject>
#include <QScriptValue>
#include <QScriptEngine>
#include "BehaviorTree.h"
#include "Blackboard.h"

using namespace ailib;

class BlackboardDecorator : public Decorator, public BlackboardListener<btHashString>
{
public:
    BlackboardDecorator(Scheduler& scheduler,
                        Behavior* child,
                        QString observedValue,
                        const Blackboard<btHashString>& blackboard);

    virtual void onValueChanged(const btHashString& key, const hold_any& value);
    virtual void run();
private:
    QString mObservedValue;
    const Blackboard<btHashString>& mBlackboard;
};

void Sequence_register_prototype(QScriptEngine& engine);
QScriptValue Sequence_prototype_ctor(QScriptContext *context, QScriptEngine *engine);

#endif // PLANNING_H
