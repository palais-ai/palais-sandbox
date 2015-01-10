#ifndef GRAPH_H
#define GRAPH_H

#pragma once

#include "ai_global.h"

#include <stdint.h>
#include <vector>
#include <cassert>

BEGIN_NS_AILIB

class Connection
{
public:
    static Connection makeConnection(uint16_t fromNode,
                                     uint16_t edgeIndex)
    {
        Connection retVal;
        retVal.fromNode = fromNode;
        retVal.edgeIndex = edgeIndex;
        return retVal;
    }

    uint16_t fromNode;
    uint16_t edgeIndex;
};

class Edge
{
public:
    static Edge makeEdge(uint16_t targetIndex,
                         real_type cost)
    {
        Edge retVal;
        retVal.cost = cost;
        retVal.targetIndex = targetIndex;
        return retVal;
    }

    real_type cost;
    uint16_t targetIndex;
};

// Stack-allocated edge-list
template <size_t MAX_EDGES>
class BaseNode
{
    STATIC_ASSERT(MAX_EDGES <= 8)
public:
    BaseNode() :
        mNumEdges(0)
    {
        ;
    }

    FORCE_INLINE const Edge* beginSuccessors() const
    {
        return &mEdges[0];
    }

    FORCE_INLINE const Edge* endSuccessors() const
    {
        return &mEdges[mNumEdges];
    }

    FORCE_INLINE void addEdge(const Edge& edge)
    {
        assert(mNumEdges <= MAX_EDGES);

        mEdges[mNumEdges++] = edge;
    }

    FORCE_INLINE size_t getNumEdges() const
    {
        return mNumEdges;
    }

private:
    Edge mEdges[MAX_EDGES];
    uint8_t mNumEdges;
};

// Dynamically allocated edge-list, default
template <>
class BaseNode<0>
{
public:
    typedef std::vector<Edge> edge_collection;

    const Edge* beginSuccessors() const
    {
        if(mEdges.size() == 0)
        {
            return NULL;
        }

        return &mEdges[0];
    }

    const Edge* endSuccessors() const
    {
        if(mEdges.size() == 0)
        {
            return NULL;
        }

        return &mEdges[0] + mEdges.size();
    }

    FORCE_INLINE void addEdge(const Edge& edge)
    {
        mEdges.push_back(edge);
    }

    FORCE_INLINE size_t getNumEdges() const
    {
        return mEdges.size();
    }
private:
    edge_collection mEdges;
};

template <typename NODE_TYPE, size_t MAX_EDGES = 0>
class Graph
{
public:
    typedef NODE_TYPE node_type;
    typedef BaseNode<MAX_EDGES> connections_type;
    typedef std::vector<node_type> node_collection;
    typedef std::vector<connections_type> connection_collection;

    FORCE_INLINE size_t addNode(const NODE_TYPE& node)
    {
        mNodes.push_back(node);
        mConnections.push_back(connections_type());

        return mNodes.size() - 1;
    }

    FORCE_INLINE void addEdge(size_t from, size_t to, real_type weight)
    {
        mConnections[from].addEdge(Edge::makeEdge(to, weight));
    }

    const NODE_TYPE* getNodesBegin() const
    {
        if(mNodes.size() == 0)
        {
            return NULL;
        }
        return &mNodes[0];
    }

    const NODE_TYPE* getNodesEnd() const
    {
        if(mNodes.size() == 0)
        {
            return NULL;
        }
        return &mNodes[0] + mNodes.size();
    }

    FORCE_INLINE const Edge* getSuccessorsBegin(size_t idx) const
    {
        return mConnections[idx].beginSuccessors();
    }

    FORCE_INLINE const Edge* getSuccessorsEnd(size_t idx) const
    {
        return mConnections[idx].endSuccessors();
    }

    FORCE_INLINE size_t getNumEdges(size_t idx) const
    {
        return mConnections[idx].getNumEdges();
    }

    FORCE_INLINE const NODE_TYPE* getNode(size_t idx) const
    {
        return &mNodes[idx];
    }

    FORCE_INLINE NODE_TYPE* getNode(size_t idx)
    {
        return &mNodes[idx];
    }

    FORCE_INLINE size_t getNumNodes() const
    {
        return mNodes.size();
    }
private:
    connection_collection mConnections;
    node_collection mNodes;
};

END_NS_AILIB

#endif // GRAPH_H
