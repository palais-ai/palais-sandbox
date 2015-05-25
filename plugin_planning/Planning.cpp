#include "Planning.h"
#include "Scene.h"
#include "Actor.h"

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
    Planner* planner = qscriptvalue_cast<Planner*>(context->thisObject());
    if(!planner)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Planner.add_action: this object is not a Planner");
    }

    QScriptValue precondition = engine->newFunction(planning_default_precondition);
    QScriptValue postcondition = engine->newFunction(planning_default_postcondition);
    QScriptValue cost = engine->newFunction(planning_default_cost);
    QScriptValue perform = engine->newFunction(planning_default_cost);
    QString name;
    for(int i = 0; i < context->argumentCount(); ++i)
    {
        QScriptValue val = context->argument(i);

        if(!val.isFunction() && i != 4)
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
        case 4:
            if(val.isString())
            {
                name = val.toString();
            }
            break;
        }
    }

    planner->addAction(new ScriptAction(precondition, postcondition, cost, perform, name));

    return engine->undefinedValue();
}

ScriptAction::ScriptAction(const QScriptValue& precondition,
                           const QScriptValue& postcondition,
                           const QScriptValue& cost,
                           const QScriptValue& perform,
                           const QString& name) :
    mPrecondition(precondition),
    mPostcondition(postcondition),
    mCost(cost),
    mPerform(perform),
    mName(name)
{
    ;
}

bool ScriptAction::isPreconditionFulfilled(const planner_state_type& state) const
{
    if(!mPrecondition.isFunction())
    {
        qWarning("ScriptAction.isPreconditionFulfilled [%s]: Function was not defined.",
                 mName.toLocal8Bit().data());
        return true;
    }

    QScriptEngine* engine = mPrecondition.engine();
    if(!engine)
    {
        qWarning("ScriptAction.isPreconditionFulfilled [%s]: "
                 "Precondition script function was invalidated.",
                 mName.toLocal8Bit().data());
        return false;
    }

    QScriptValue value = engine->toScriptValue<QVariantMap>(QVariantMap(state));
    QScriptValue retVal = mPrecondition.call(QScriptValue(), QScriptValueList() << value);

    if(retVal.isUndefined() || retVal.isNull())
    {
        qWarning("ScriptAction.isPreconditionFulfilled [%s]: "
                 "Script precondition didn't give a valid return value.",
                 mName.toLocal8Bit().data());
        return false;
    }

    bool returnCode = retVal.toBool();
    return returnCode;
}

void ScriptAction::applyPostcondition(planner_state_type& state) const
{
    if(!mPostcondition.isFunction())
    {
        qWarning("ScriptAction.applyPostcondition [%s]: Function was not defined.",
                 mName.toLocal8Bit().data());
    }

    QScriptValue value = mPostcondition.engine()->toScriptValue(state);
    state = mPostcondition.call(QScriptValue(), QScriptValueList() << value).toVariant().toMap();
}

float ScriptAction::getCost(const planner_state_type& state) const
{
    if(!mCost.isFunction())
    {
        qWarning("ScriptAction.getCost [%s]: Function was not defined.",
                 mName.toLocal8Bit().data());
        return 0;
    }

    QScriptValue stateValue = mCost.engine()->toScriptValue(state);
    QScriptValue costValue = mCost.call(QScriptValue(), QScriptValueList() << stateValue);
    if(costValue.isNumber())
    {
        return costValue.toNumber();
    }
    else
    {
        qWarning("ScriptAction.getCost [%s]: Cost function returned a value that "
                 "wasn't a number. Assuming 0.", mName.toLocal8Bit().data());
        return 0;
    }
}

void ScriptAction::perform(Actor* actor)
{
    if(!mPerform.isFunction())
    {
        qWarning("ScriptAction.perform [%s]: Function was not defined.",
                 mName.toLocal8Bit().data());
    }

    QScriptValue value = mPerform.engine()->toScriptValue(actor);
    mPerform.call(QScriptValue(), QScriptValueList() << value);
}

QString ScriptAction::getName() const
{
    return mName;
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

QStringList Planner::findPlan(const QVariantMap& startState,
                              const QVariantMap& endState,
                              int maxDepth)
{
    bool isAlreadyThere;
    ailib::AStar<planner_type::graph_type>::connections_type connections;
    connections = findPlan(startState, endState, &isAlreadyThere, maxDepth);

    if(connections.empty())
    {
        qDebug("Planner.findPlan: No possible solution found.");
    }
    else if(isAlreadyThere)
    {
        qDebug("Planner.findPlan: Plan's goal has already been reached.");
    }
    else
    {
        qDebug() << "Planner.makePlan: Found planning solutions with "
                 << connections.size() << " steps.";

        const planner_type::graph_type& graph = mPlanner.getGraph();
        ailib::AStar<planner_type::graph_type>::connections_type::const_iterator it;
        QStringList names;
        for(it = connections.begin(); it != connections.end(); ++it)
        {
            ScriptAction* action = static_cast<ScriptAction*>((graph.getSuccessorsBegin(it->fromNode) +
                                                               it->edgeIndex)->userData);

            if(action)
            {
                names.append(action->getName());
            }
            else
            {
                qWarning("Planner.findPlan: ScriptAction wasn't set.");
            }
        }
        return names;
    }
    return QStringList();
}

ailib::AStar<Planner::planner_type::graph_type>::connections_type
Planner::findPlan(const QVariantMap& startState,
                  const QVariantMap& endState,
                  bool* isAlreadyThere,
                  int maxDepth)
{
    size_t startIdx = mPlanner.buildGraph(startState, endState, maxDepth);

    qDebug() << "Planner.findPlan: Built a planning state graph with "
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
        qWarning() << "Planner.findPlan: Could not make a plan that leads to the desired end state."
                   << "Start state: " << startState
                   << "End state: "   << endState;
    }

    if(isAlreadyThere)
    {
        *isAlreadyThere = path.size() == 1 && *path[0] == startState;
    }

    return connections;
}

void Planner::makePlan(Actor* actor,
                       const planner_state_type& endState,
                       int32_t maxDepth)
{
    const QVariantMap& knowledge = actor->getKnowledge();

    QStringList names = findPlan(knowledge, endState, maxDepth);

    if(!names.empty())
    {
        // TODO: Kickoff to sequentially perform actions,
        // recalculate the plan if unexpected state changes happen.
        qDebug() << "Planner.makePlan: Found planning solutions with "
                 << names.size() << " steps.";

        actor->setKnowledge("plan", names);
    }
    else
    {
        actor->removeKnowledge("plan");
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

