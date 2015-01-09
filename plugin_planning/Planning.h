#ifndef PLANNING_H
#define PLANNING_H

#include <QObject>
#include <QScriptValue>
#include <QVector>
#include <QVariant>

#include "GOAP.h"
#include "AStar.h"

class Actor;
class Scene;
class QScriptEngine;
class QScriptContext;

typedef QVariantMap planner_state_type;

QScriptValue planning_add_action(QScriptContext *context, QScriptEngine *engine);
QScriptValue planning_default_precondition(QScriptContext *context, QScriptEngine *engine);
QScriptValue planning_default_postcondition(QScriptContext *context, QScriptEngine *engine);
QScriptValue planning_default_cost(QScriptContext *context, QScriptEngine *engine);

void Planner_register_prototype(QScriptEngine& engine);
QScriptValue Planner_prototype_ctor(QScriptContext *context, QScriptEngine *engine);

class ScriptAction : public ailib::Action<planner_state_type>
{
public:
    ScriptAction(const QScriptValue& precondition,
                 const QScriptValue& postcondition,
                 const QScriptValue& cost);

    virtual bool isPreconditionFulfilled(const planner_state_type& state) const;
    virtual void applyPostcondition(planner_state_type& state) const;
    virtual float getCost(const planner_state_type& state) const;
private:
    mutable QScriptValue mPrecondition, mPostcondition, mCost;
};

class Planner : public QObject
{
    Q_OBJECT
public:
    typedef ailib::GOAPPlanner<planner_state_type> planner_type;

    explicit Planner(QObject *parent = 0);
    ~Planner();

    Q_INVOKABLE void makePlan(Actor* actor,
                              const QVariantMap& endState);

    ailib::AStar<planner_type::graph_type>::path_type findPlan(const QVariantMap& startState,
                                                               const QVariantMap& endState,
                                                               bool* isAlreadyThere);

    void addAction(ScriptAction* action);
    void update(Scene& scene, float deltaTime);
private:
    planner_type mPlanner;
    QVector<ScriptAction*> mActions;
};

class Planning : public QObject
{
    Q_OBJECT
public:
    static void onDestroyed(QObject* obj);
    static void update(Scene& scene, float deltaTime);
private:
    static QVector<Planner*> sPlanners;
};

#endif // PLANNING_H
