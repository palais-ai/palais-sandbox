#ifndef BEHAVIOR_P_H
#define BEHAVIOR_P_H

#include "BehaviorTree.h"

#include <QObject>
#include <QSharedPointer>
#include <QScriptValue>
#include <QVariant>
#include <QScriptable>
#include <QHash>
#include <QDebug>

using namespace ailib;

class ScriptBehavior : public Behavior
{
public:
    ScriptBehavior(const QScriptValue& obj);
    virtual ~ScriptBehavior();

    virtual void run();
    virtual void terminate();

private:
    QScriptValue mScript;
};

class BehaviorPrototype : public QObject, public QScriptable
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap userData READ getUserData)
public:
    explicit BehaviorPrototype(QObject* parent = NULL);

    Q_INVOKABLE void setUserData(QVariantMap* data);
    Q_INVOKABLE QVariantMap getUserData() const;
    Q_INVOKABLE void setStatus(int status);
    void setStatus(Status status);
    Q_INVOKABLE Status getStatus() const;
    Q_INVOKABLE void notifySuccess();
    Q_INVOKABLE void notifyFailure();
    Q_INVOKABLE void notifyReset();
};

class SchedulerPrototype : public QObject, public QScriptable
{
    Q_OBJECT
public:
    explicit SchedulerPrototype(QObject* parent = NULL);

    Q_INVOKABLE void enqueue(QScriptValue behaviorValue);
    Q_INVOKABLE void dequeue(QScriptValue behaviorValue);
private:
    QHash<qint64, QScriptValue> mActiveBehaviors;
    QHash<qint64, Behavior*> mAddedBehaviors;
};

#endif // BEHAVIOR_P_H

