#ifndef GOAP_H
#define GOAP_H

#pragma once

#include "ai_global.h"
#include "Graph.h"

BEGIN_NS_AILIB

template <typename STATE>
class Action
{
public:
    typedef STATE state_type;

    virtual ~Action() {}

    virtual bool isPreconditionFulfilled(const state_type& state) const = 0;
    virtual void applyPostcondition(state_type& state) const = 0;
    virtual float getCost(const state_type& state) const = 0;
};

template <typename STATE, size_t MAX_ACTIONS = 0>
class GOAPPlanner
{
public:
    typedef STATE state_type;
    typedef Action<state_type> action_type;
    typedef Graph<state_type, MAX_ACTIONS> graph_type;

    void addAction(action_type* action)
    {
        mActions.push_back(action);
    }

    const graph_type& getGraph() const
    {
        return mGraph;
    }

    size_t buildGraph(const state_type& startState, const state_type& endState, uint32_t maxDepth)
    {
        mGraph = graph_type(); //< Clear before build
        size_t startIdx = mGraph.addNode(startState);
        recursiveBuildGraph(startIdx, maxDepth, 0);
        return startIdx;
    }
private:
    void recursiveBuildGraph(size_t currentIdx,
                             uint32_t maxDepth,
                             uint32_t currentDepth)
    {
        const state_type currentState = *mGraph.getNode(currentIdx);

        for(typename std::vector<action_type*>::const_iterator it = mActions.begin(); it != mActions.end(); ++it)
        {
            const action_type* action = *it;

            if(action->isPreconditionFulfilled(currentState))
            {
                state_type nextState = currentState;
                action->applyPostcondition(nextState);
                const size_t nextIdx = mGraph.addNode(nextState);
                mGraph.addEdge(currentIdx, nextIdx, action->getCost(currentState));

                if(currentDepth < maxDepth)
                {
                    recursiveBuildGraph(nextIdx, maxDepth, currentDepth + 1);
                }
            }
        }
    }

    graph_type mGraph;
    std::vector<action_type*> mActions;
};

END_NS_AILIB

#endif // GOAP_H
