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
                 const QScriptValue& cost,
                 const QScriptValue& perform,
                 const QString& name);

    virtual bool isPreconditionFulfilled(const planner_state_type& state) const;
    virtual void applyPostcondition(planner_state_type& state) const;
    virtual float getCost(const planner_state_type& state) const;

    void perform(Actor* actor);

    QString getName() const;
private:
    mutable QScriptValue mPrecondition, mPostcondition, mCost, mPerform;
    QString mName;
};

template <typename T>
struct PlanningHash;

template <>
struct PlanningHash<planner_state_type>
{
    uint64_t operator()(const planner_state_type& state) const
    {
        // CREDITS: FNV-1a algorithm primes from http://www.isthe.com/chongo/tech/comp/fnv/
        // (public domain)
        static const uint64_t fnv_prime    =        1099511628211u;
        static const uint64_t offset_basis = 14695981039346656037u;

        uint64_t hash = offset_basis;
        // Combine the hashes of the individual keys.
        for(planner_state_type::const_iterator it = state.begin(); it != state.end(); ++it)
        {
            hash *= fnv_prime;
            hash += qHash(it.key());
        }
        return hash;
    }
};

class Planner : public QObject
{
    Q_OBJECT
public:
    typedef ailib::GOAPPlanner<planner_state_type, 0, PlanningHash<planner_state_type> >
            planner_type;

    explicit Planner(QObject *parent = 0);
    ~Planner();

    Q_INVOKABLE void makePlan(Actor* actor,
                              const QVariantMap& endState);

    ailib::AStar<planner_type::graph_type>::connections_type findPlan(const QVariantMap& startState,
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
