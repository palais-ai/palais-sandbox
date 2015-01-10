#include "Planning.h"
#include "scene.h"
#include "actor.h"

#include <QScriptEngine>
#include <QDebug>

Q_DECLARE_METATYPE(Planner*)

QVector<Planner*> Planning::sPlanners;

QScriptValue Planner_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);

    Planner* planner = new Planner();
    QObject::connect(planner, &QObject::destroyed, &Planning::onDestroyed);

    QScriptValue value = engine->newQObject(planner, QScriptEngine::ScriptOwnership);
    value.setProperty("addAction", engine->newFunction(planning_add_action));
    value.setProperty("defaultPrecondition", engine->newFunction(planning_default_precondition));
    value.setProperty("defaultPostCondition", engine->newFunction(planning_default_postcondition));
    value.setProperty("defaultCost", engine->newFunction(planning_default_cost));

    return value;
}

void Planner_register_prototype(QScriptEngine& engine)
{
    const int typeId = qRegisterMetaType<Planner*>("Planner*");

    QScriptValue prototype = engine.newQObject((Planner*)0);
    prototype.setProperty("addAction", engine.newFunction(planning_add_action));
    engine.setDefaultPrototype(typeId, prototype);

    engine.globalObject().setProperty("Planner",
                                      engine.newFunction(Planner_prototype_ctor));
}

QScriptValue planning_default_precondition(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);
    Q_UNUSED(engine);
    return true;
}

QScriptValue planning_default_postcondition(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);
    return context->argument(0);
}

 QScriptValue planning_default_cost(QScriptContext *context, QScriptEngine *engine)
{
     Q_UNUSED(context);
     Q_UNUSED(engine);
    return 1;
}

QScriptValue planning_add_action(QScriptContext *context, QScriptEngine *engine)
{
    // Cast to a pointer to be able to modify the underlying C++ value
    Planner* planner = qscriptvalue_cast<Planner*>(context->thisObject());

    if(!planner)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Planner.add_action: this object is not a Planner");
    }

    QScriptValue precondition, postcondition, cost;
    if(context->argumentCount() > 0)
    {
        QScriptValue arg0 = context->argument(0);
        if(arg0.isFunction())
        {
            precondition = arg0;
        }
        else
        {
            qWarning() << "The first argument to Planner.addAction must be a function.";
            return engine->undefinedValue();
        }

        if(context->argumentCount() > 1)
        {
            QScriptValue arg1 = context->argument(1);
            if(arg1.isFunction())
            {
                postcondition = arg1;
            }
            else
            {
                qWarning() << "The second argument to Planner.addAction must be a function.";
                return engine->undefinedValue();
            }

            if(context->argumentCount() > 2)
            {
                QScriptValue arg2 = context->argument(2);
                if(arg0.isFunction())
                {
                    cost = arg2;
                }
                else
                {
                    qWarning() << "The third argument to Planner.addAction must be a function.";
                    return engine->undefinedValue();
                }
            }
        }
    }
    else
    {
        precondition = engine->newFunction(planning_default_precondition);
        postcondition = engine->newFunction(planning_default_postcondition);
        cost = engine->newFunction(planning_default_cost);
    }

    planner->addAction(new ScriptAction(precondition, postcondition, cost));

    return engine->undefinedValue();
}

ScriptAction::ScriptAction(const QScriptValue& precondition,
                           const QScriptValue& postcondition,
                           const QScriptValue& cost) :
    mPrecondition(precondition),
    mPostcondition(postcondition),
    mCost(cost)
{
    ;
}

bool ScriptAction::isPreconditionFulfilled(const planner_state_type& state) const
{
    QScriptEngine* engine = mPrecondition.engine();

    if(!engine)
    {
        qWarning("Precondition script function was invalidated.");
        return false;
    }

    QScriptValue value = engine->toScriptValue<QVariantMap>(QVariantMap(state));
    QScriptValue retVal = mPrecondition.call(QScriptValue(), QScriptValueList() << value);

    if(retVal.isUndefined() || retVal.isNull())
    {
        qWarning("Script precondition didn't give a valid return value.");
        return false;
    }

    bool returnCode = retVal.toBool();
    return returnCode;
}

void ScriptAction::applyPostcondition(planner_state_type& state) const
{
    QScriptValue value = mPrecondition.engine()->toScriptValue(state);
    state = mPostcondition.call(QScriptValue(), QScriptValueList() << value).toVariant().toMap();
}

float ScriptAction::getCost(const planner_state_type& state) const
{
    QScriptValue value = mPrecondition.engine()->toScriptValue(state);
    return mPostcondition.call(QScriptValue(), QScriptValueList() << value).toNumber();
}

Planner::Planner(QObject *parent) :
    QObject(parent)
{
    ;
}

Planner::~Planner()
{
    foreach(ScriptAction* action, mActions)
    {
        delete action;
    }
}

void Planner::addAction(ScriptAction* action)
{
    mActions += action;
    mPlanner.addAction(action);
}

ailib::AStar<Planner::planner_type::graph_type>::path_type
Planner::findPlan(const QVariantMap& startState,
                  const QVariantMap& endState,
                  bool* isAlreadyThere)
{
    const int32_t maxDepth = 5;
    size_t startIdx = mPlanner.buildGraph(startState, endState, maxDepth);

    qDebug() << "[PLANNER] Built a planning state graph with "
             << mPlanner.getGraph().getNumNodes()
             << " nodes. (maxdepth="
             << maxDepth
             << ")";

    ailib::AStar<planner_type::graph_type> astar(mPlanner.getGraph());
    ailib::AStar<planner_type::graph_type>::path_type path;
    path = astar.findPath(mPlanner.getGraph().getNode(startIdx), endState);

    if(path.empty())
    {
        qWarning("Could not make a plan that leads to the desired end state.");
        qWarning() << "Start state: " << startState;
        qWarning() << "End state: " << endState;
    }

    if(isAlreadyThere)
    {
        *isAlreadyThere = path.size() == 1 && *path[0] == startState;
    }

    return path;
}

void Planner::makePlan(Actor* actor,
                       const planner_state_type& endState)
{
    QVariantMap knowledge = actor->getKnowledge();

    bool isAlreadyThere;
    ailib::AStar<planner_type::graph_type>::path_type path;
    path = findPlan(knowledge, endState, &isAlreadyThere);

    if(path.empty())
    {
        qDebug("No possible solution found.");
    }
    else if(isAlreadyThere)
    {
        qDebug("Goal has already been reached.");
    }
    else
    {
        // TODO: Kickoff to sequentially perform actions,
        // recalculate the plan if unexpected state changes happen.
        qDebug() << "[PLANNER] Found planning solutions with "
                 << path.size() << " steps.";
    }
}

void Planner::update(Scene& scene, float deltaTime)
{
    Q_UNUSED(scene);
    Q_UNUSED(deltaTime);
}

void Planning::onDestroyed(QObject* obj)
{
    int idx = sPlanners.indexOf(static_cast<Planner*>(obj));

    if(idx != -1)
    {
        sPlanners.remove(idx);
    }
}

void Planning::update(Scene& scene, float deltaTime)
{
    foreach(Planner* planner, sPlanners)
    {
        planner->update(scene, deltaTime);
    }
}

