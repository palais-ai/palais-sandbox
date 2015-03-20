#ifndef IDASTAR_H
#define IDASTAR_H

#pragma once

#include "ai_global.h"
#include "graph.h"
#include <limits>

BEGIN_NS_AILIB

template <typename GRAPH>
class IDAStar
{
private:
    const GRAPH& mGraph;
public:
    typedef typename GRAPH::node_type node_type;
    typedef std::vector<const node_type*> path_type;
    typedef real_type(*Heuristic)(const node_type&,
                                  const node_type&);

    IDAStar(const GRAPH& graph) :
        mGraph(graph)
    {
        ;
    }

    path_type findPath(const node_type* const start,
                       const node_type* const goal,
                       Heuristic heuristic,
                       uint32_t maxDepth) const
    {
        assert(start);
        assert(goal);

        real_type estimate = heuristic(*start, *goal);

        while(true)
        {
            real_type newEstimate;
            DepthSearchResult res = depthSearch(start, goal, 0, estimate, heuristic, newEstimate);

            if(res == DepthSearchResultFound)
            {
                return path_type();
            }
            else if(res == DepthSearchResultUnreachable)
            {
                return path_type();
            }

            estimate = newEstimate;
        }
    }
private:
    enum DepthSearchResult
    {
        DepthSearchResultFound = 0,
        DepthSearchResultUnreachable,
        DepthSearchResultCost
    };

    DepthSearchResult depthSearch(const node_type* node,
                                  const node_type* const goal,
                                  real_type currentCost,
                                  real_type currentEstimate,
                                  Heuristic heuristic,
                                  real_type& /* out */ nextEstimate) const
    {
        real_type estimate = currentCost + heuristic(*node, *goal);

        if(estimate > currentEstimate)
        {
            nextEstimate = estimate;
            return DepthSearchResultCost;
        }

        if(node == goal)
        {
            return DepthSearchResultFound;
        }

        const node_type* const firstNode = mGraph.getNodesBegin();
        const size_t nodeIdx = node - firstNode;
        assert(nodeIdx < mGraph.getNumNodes());

        real_type min = std::numeric_limits<real_type>::max();
        const Edge* const end = mGraph.getSuccessorsEnd(nodeIdx);
        for(const Edge* it = mGraph.getSuccessorsBegin(nodeIdx); it != end; ++it)
        {
            real_type tmp = std::numeric_limits<real_type>::max();
            DepthSearchResult res = depthSearch(mGraph.getNode(it->targetIndex),
                                                currentCost + it->cost,
                                                currentEstimate,
                                                heuristic,
                                                tmp);

            if(res == DepthSearchResultFound)
            {
                return DepthSearchResultFound;
            }
            else if(tmp < min)
            {
                min = tmp;
            }
        }

        if(min == std::numeric_limits<real_type>::max())
        {
            return DepthSearchResultUnreachable;
        }
        else
        {
            nextEstimate = min;
            return DepthSearchResultCost;
        }
    }
};

END_NS_AILIB

#endif // IDASTAR_H
