#ifndef ASTAR_H
#define ASTAR_H

#pragma once

#include "ai_global.h"
#include "graph.h"

#include <cstring>
#include <cassert>
#include <algorithm>
#include <queue>

BEGIN_NS_AILIB

template <typename GRAPH>
class AStar
{
private:
    // AStarNode carries the bookkeeping information necessary for the A* algorithm.
    class AStarNode
    {
    public:
        enum NodeState
        {
            NodeStateUnvisited = 0,
            NodeStateClosed,
            NodeStateOpen
        };

        real_type estTotalCost;
        real_type currentCost;
        AStarNode* parent;
        uint16_t connection;
        NodeState state;
    };

    class NodeComparator
    {
    public:
        FORCE_INLINE bool operator() (const AStarNode* lv, const AStarNode* rv) const
        {
            // Greather than ('>') because priority_queue orders from highest to lowest.
            // We require the opposite.
            return lv->estTotalCost > rv->estTotalCost;
        }
    };

    const GRAPH& mGraph;

    // Mutable, because it's a cache
    mutable std::vector<AStarNode> mNodeInfo;
public:
    typedef typename GRAPH::node_type node_type;
    typedef std::vector<const node_type*> path_type;
    typedef std::vector<Connection> connections_type;
    typedef real_type(*Heuristic)(const node_type&,
                                  const node_type&);

    static real_type zeroHeuristic(const node_type&,
                            const node_type&)
    {
        return 0;
    }

    AStar(const GRAPH& staticGraph) :
        mGraph(staticGraph),
        // Pre-allocate bookkeeping information for every node
        mNodeInfo(mGraph.getNumNodes())
    {
        ;
    }

    path_type findPath(const node_type* const start,
                       const node_type& goal,
                       Heuristic heuristic = &zeroHeuristic,
                       connections_type* /* out */ connections = NULL) const
    {
        assert(start);

        // Make sure there is enough space in the node cache to handle all nodes.
        // This is necessary if the graph has changed its size in between construction
        // and this path query.
        mNodeInfo.reserve(mGraph.getNumNodes());

        std::priority_queue<AStarNode*,
                            std::vector<AStarNode*>,
                            NodeComparator> open;

        // Zero-initialize the bookkeeping information
        std::memset(&mNodeInfo[0],
                    0,
                    mNodeInfo.size() * sizeof(AStarNode));

        const node_type* const firstNode = mGraph.getNodesBegin();
        const AStarNode* const firstNodeInfo = &mNodeInfo[0];

        const size_t startIdx = start - firstNode;
        assert(startIdx < mGraph.getNumNodes());

        AStarNode* startNode = &mNodeInfo[startIdx];
        startNode->estTotalCost = heuristic(*start, goal);
        startNode->state = AStarNode::NodeStateOpen;
        open.push(startNode);

        while(LIKELY(!open.empty()))
        {
            AStarNode* lowestCostNode = open.top();

            const size_t lowestCostIdx = lowestCostNode - firstNodeInfo;
            assert(lowestCostIdx < mGraph.getNumNodes());

            const node_type* lowestCost = mGraph.getNode(lowestCostIdx);
            if(UNLIKELY(*lowestCost == goal))
            {
                // We found a valid short path.
                // It's guaranteed to be the shortest, if our heuristic is underestimating.
                return buildPath(lowestCostNode, startIdx, connections);
            }

            lowestCostNode->state = AStarNode::NodeStateClosed;

            // The lowest cost node is going to be processed and removed from the open list.
            // We have to remove it before adding any children in case they have
            // a better cost value.
            open.pop();

            const Edge* const end = mGraph.getSuccessorsEnd(lowestCostIdx);
            const Edge* const begin = mGraph.getSuccessorsBegin(lowestCostIdx);
            for(const Edge* it = begin; it != end; ++it)
            {
                const size_t targetIdx = it->targetIndex;
                assert(targetIdx < mGraph.getNumNodes());

                real_type targetCost = lowestCostNode->currentCost + it->cost;
                real_type heuristicValue = 0.;

                AStarNode* targetNode = &mNodeInfo[targetIdx];

                if(targetNode->state == AStarNode::NodeStateUnvisited)
                {
                    const node_type* target = mGraph.getNode(targetIdx);

                    // We have to calculate the heuristic for unvisited nodes.
                    heuristicValue = heuristic(*target, goal);

                    targetNode->estTotalCost = targetCost + heuristicValue;
                    targetNode->state = AStarNode::NodeStateOpen;
                    open.push(targetNode);
                }
                else
                {
                    if(LIKELY(targetNode->currentCost <= targetCost))
                    {
                        // Continue if this node doesn't offer improvement
                        continue;
                    }

                    // Reuse the heuristic value
                    heuristicValue = targetNode->estTotalCost - targetNode->currentCost;

                    targetNode->estTotalCost = targetCost + heuristicValue;
                    if(targetNode->state != AStarNode::NodeStateOpen)
                    {
                        targetNode->state = AStarNode::NodeStateOpen;
                        open.push(targetNode);
                    }
                }

                targetNode->parent = lowestCostNode;
                targetNode->connection = it - begin;
                targetNode->currentCost = targetCost;
            }
        }

        // No solution found. Return an empty path.
        return path_type();
    }

private:
    path_type buildPath(const AStarNode* fromNode,
                        const size_t startIdx,
                        connections_type* connections) const
    {
        const AStarNode* const firstNodeInfo = &mNodeInfo[0];
        const AStarNode* const startNode = &mNodeInfo[startIdx];
        const node_type* const start = mGraph.getNode(startIdx);

        path_type retVal;

        const AStarNode* currentNode = fromNode;
        while(currentNode != startNode)
        {
            const size_t currentIdx = currentNode - firstNodeInfo;
            assert(currentIdx < mGraph.getNumNodes());

            const node_type* current = mGraph.getNode(currentIdx);
            retVal.push_back(current);
            currentNode = currentNode->parent;

            if(connections)
            {
                const size_t parentIdx = currentNode - firstNodeInfo;
                assert(parentIdx < mGraph.getNumNodes());
                connections->push_back(Connection::makeConnection(parentIdx, currentNode->connection));
            }
        }

        retVal.push_back(start);

        // Reverse the path so it is in order from __start__ to __goal__
        return path_type(retVal.rbegin(), retVal.rend());
    }
};

END_NS_AILIB

#endif // ASTAR_H