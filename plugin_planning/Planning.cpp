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

    QScriptValue precondition, postcondition, cost, perform;
    if(context->argumentCount() >= 4)
    {
        for(int i = 0; i < 4; ++i)
        {
            QScriptValue val = context->argument(i);

            if(!val.isFunction())
            {
                return context->throwError(QScriptContext::TypeError,
                                           "Planner.add_action: arguments must be functions.");
            }

            switch(i)
            {
            case 0:
                precondition = val;
                break;
            case 1:
                postcondition = val;
                break;
            case 2:
                cost = val;
                break;
            case 3:
                perform = val;
                break;
            }
        }
    }
    else
    {
        precondition = engine->newFunction(planning_default_precondition);
        postcondition = engine->newFunction(planning_default_postcondition);
        cost = engine->newFunction(planning_default_cost);
        perform = engine->newFunction(planning_default_cost);
    }

    planner->addAction(new ScriptAction(precondition, postcondition, cost, perform));

    return engine->undefinedValue();
}

ScriptAction::ScriptAction(const QScriptValue& precondition,
                           const QScriptValue& postcondition,
                           const QScriptValue& cost,
                           const QScriptValue& perform) :
    mPrecondition(precondition),
    mPostcondition(postcondition),
    mCost(cost),
    mPerform(perform)
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

void ScriptAction::perform(Actor* actor)
{
    QScriptValue value = mPerform.engine()->toScriptValue(actor);
    mPerform.call(QScriptValue(), QScriptValueList() << value);
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

static bool variantMapComparator(const QVariantMap& node,
                                 const QVariantMap& goal)
{
    QVariantMap::const_iterator it = goal.constBegin();

    for(; it != goal.end(); ++it)
    {
        if(node.contains(it.key()) && node[it.key()] == it.value())
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}

ailib::AStar<Planner::planner_type::graph_type>::connections_type
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

    ailib::AStar<planner_type::graph_type>::connections_type connections;
    ailib::AStar<planner_type::graph_type> astar(mPlanner.getGraph());
    ailib::AStar<planner_type::graph_type>::path_type path;
    path = astar.findPath(mPlanner.getGraph().getNode(startIdx),
                          endState,
                          &variantMapComparator,
                          &connections);

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

    return connections;
}

void Planner::makePlan(Actor* actor,
                       const planner_state_type& endState)
{
    QVariantMap knowledge = actor->getKnowledge();

    bool isAlreadyThere;
    ailib::AStar<planner_type::graph_type>::connections_type connections;
    connections = findPlan(knowledge, endState, &isAlreadyThere);

    if(connections.empty())
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
                 << connections.size() << " steps.";

        const planner_type::graph_type& graph = mPlanner.getGraph();
        ailib::AStar<planner_type::graph_type>::connections_type::const_iterator it;
        for(it = connections.begin(); it != connections.end(); ++it)
        {
            qDebug() << it->edgeIndex;
            qDebug() << graph.getNumEdges(it->fromNode);
            static_cast<ScriptAction*>((graph.getSuccessorsBegin(it->fromNode) +
                                        it->edgeIndex)->userData)
                                      ->perform(actor);
        }
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

