#include "pathfinding.h"
#include "ogrehelper.h"
#include "DebugDrawer.h"
#include "utility/MetatypeDeclarations.h"
#include "actor.h"
#include "scene.h"

#include <QScopedArrayPointer>
#include <QHash>
#include <QVector>
#include <QSet>
#include <QDebug>

#include <OgreLogManager.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <OgreMovableObject.h>
#include <OgrePlane.h>
#include <OgrePolygon.h>
#include <OgreStringConverter.h>

static ailib::real_type euclideanHeuristic(const Pathfinding::NavigationGraph::node_type& n1,
                                           const Pathfinding::NavigationGraph::node_type& n2)
{
    return n1.getCentroid().squaredDistance(n2.getCentroid());
}

void Pathfinding::updateActor(Actor &actor, float deltaTime)
{
    static const float defaultActorSpeed = 0.5;
    if(actor.hasKnowledge("movement_target"))
    {
        float actorSpeed;
        if(actor.hasKnowledge("actor_speed"))
        {
            bool ok;
            actorSpeed = actor.getKnowledge("actor_speed").toFloat(&ok);

            if(!ok)
            {
                actorSpeed = defaultActorSpeed;
            }
        }
        else
        {
            actorSpeed = defaultActorSpeed;
        }

        Ogre::Vector3 target = actor.getKnowledge("movement_target").value<Ogre::Vector3>();
        const Ogre::Vector3& current = actor.getPosition();

        if(current.distance(target) < 0.01)
        {
            if(actor.hasKnowledge("current_path"))
            {
                QVector<Ogre::Vector3> path =
                        actor.getKnowledge("current_path").value<QVector<Ogre::Vector3> >();
                Ogre::Vector3 next = path[0];

                if(path.size() == 1)
                {
                    actor.removeKnowledge("current_path");
                }
                else
                {
                    path.removeFirst();
                    actor.setKnowledge("current_path", QVariant::fromValue(path));
                }

                actor.setKnowledge("movement_target", QVariant::fromValue(next));
                target = next;
            }
            else
            {
                actor.removeKnowledge("movement_target");
                actor.disableAnimation("my_animation");
                qDebug("Reached goal.");
            }
        }

        Ogre::Vector3 step = (target - current).normalisedCopy() * (actorSpeed * deltaTime);

        actor.setPosition(current + step);
    }
}

void Pathfinding::update(Scene& scene, float deltaTime)
{
    QMapIterator<QString, Actor*> it(scene.getActors());
    while(it.hasNext())
    {
        it.next();

        updateActor(*it.value(), deltaTime);
    }
}

class Edge
{
public:
    Edge(unsigned i1, unsigned i2) :
        index1(i1 < i2 ? i1 : i2),
        index2(i1 < i2 ? i2 : i1)
    {
        ;
    }

    unsigned index1, index2;

    bool operator==(const Edge& other) const
    {
        return index1 == other.index1 && index2 == other.index2;
    }
};

static uint qHash(const Edge& key)
{
     return key.index1 ^ key.index2;
}

Triangle::Triangle(const Ogre::Vector3& v1,
                   const Ogre::Vector3& v2,
                   const Ogre::Vector3& v3) :
    a(v1),
    b(v2),
    c(v3),
    centroid(computeCentroid())
{
    ;
}

const Ogre::Vector3& Triangle::getCentroid() const
{
    return centroid;
}

Ogre::Vector3 Triangle::computeCentroid()
{
    const float lenA = c.squaredDistance(b),
                lenB = a.squaredDistance(c),
                lenC = b.squaredDistance(a);

    const float mul = sqrtf(lenA*lenB*lenC);
    return fromBarycentric(mul, mul, mul);
}

Ogre::Vector3 Triangle::fromBarycentric(float x, float y, float z) const
{
    const float div = x+y+z;
    return (x / div) * a + (y / div) * b + (z / div) * c;
}

bool Triangle::isProjectionInside(const Ogre::Vector3& point) const
{
    Ogre::Plane triPlane(a, b, c);
    triPlane.normalise();

    Ogre::Vector3 pt = triPlane.projectVector(point);

    const float sign = sinf((b-a).angleBetween(c-a).valueRadians());

    Ogre::Polygon poly;
    // Counterclockwise
    if(sign > 0.f)
    {
        poly.insertVertex(a);
        poly.insertVertex(b);
        poly.insertVertex(c);
    }
    // Clockwise
    else
    {
        poly.insertVertex(c);
        poly.insertVertex(b);
        poly.insertVertex(a);
    }

    // TODO: Improve projection so this hack isnt necessary.
    pt.y = a.y;

    return poly.isPointInside(pt);
}

const Pathfinding::NavigationGraph::node_type*
Pathfinding::getNavNodeClosestToPoint(const Ogre::Vector3& point)
{
    const NavigationGraph::node_type* it = mGraph.getNodesBegin();
    for(; it != mGraph.getNodesEnd(); ++it)
    {
        const NavigationGraph::node_type& node = *it;

        if(node.isProjectionInside(point))
        {
           return &node;
        }
    }

    return NULL;
}

ailib::AStar<Pathfinding::NavigationGraph>::path_type
Pathfinding::planPath(const Ogre::Vector3& from,
                      const Ogre::Vector3& to,
                      bool* isAlreadyThere)
{
    const NavigationGraph::node_type* start = getNavNodeClosestToPoint(from);
    if(!start)
    {
        qWarning() << "Target position "
                   << Ogre::StringConverter::toString(from).c_str()
                   << " is not covered by the navmesh. No valid path could be calculated.";
        return ailib::AStar<NavigationGraph>::path_type();
    }

    const NavigationGraph::node_type* goal = getNavNodeClosestToPoint(to);
    if(!goal)
    {
        qWarning() << "Target position "
                   << Ogre::StringConverter::toString(to).c_str()
                   << " is not covered by the navmesh. No valid path could be calculated.";
        return ailib::AStar<NavigationGraph>::path_type();
    }

    ailib::AStar<NavigationGraph> astar(mGraph);
    ailib::AStar<NavigationGraph>::path_type path = astar.findPath(start, goal, euclideanHeuristic);

    if(isAlreadyThere)
    {
        *isAlreadyThere = path.size() == 1 && path[0] == start;
    }

    return path;
}

void
Pathfinding::initNavGraphFromOgreNode(Ogre::SceneNode* node)
{
    Pathfinding::NavigationGraph graph;
    ailib::AStar<Pathfinding::NavigationGraph>::Heuristic heuristic = euclideanHeuristic;

    if(node->numAttachedObjects() > 0)
    {
        const Ogre::MeshPtr before = static_cast<Ogre::Entity*>(node->getAttachedObject(0))
                                                                ->getMesh();
        const Ogre::MeshPtr mesh = OgreHelper::optimizeMesh(before);

        if(!mesh.get())
        {
            qWarning() << "Optimized mesh ptr was null. Returning empty nav graph.";
            mGraph = graph;
            return;
        }

        size_t vertexCount, indexCount;
        Ogre::Vector3* verticesPtr;
        unsigned* indicesPtr;
        OgreHelper::getMeshInformation(mesh,
                                       vertexCount,
                                       verticesPtr,
                                       indexCount,
                                       indicesPtr,
                                       node->getPosition(),
                                       node->getOrientation(),
                                       node->getScale());

        QScopedArrayPointer<Ogre::Vector3> vertices(verticesPtr);
        QScopedArrayPointer<unsigned> indices(indicesPtr);

        QHash<Edge, QSet<uint32_t> > edgeConnections;
        for(size_t i = 0; i < indexCount; i+=3)
        {
            uint32_t idx = graph.addNode(Pathfinding::NavigationGraph::node_type(vertices[indices[i]],
                                                                                 vertices[indices[i+1]],
                                                                                 vertices[indices[i+2]]));

            edgeConnections[Edge(indices[i],   indices[i+1])] += idx;
            edgeConnections[Edge(indices[i+1], indices[i+2])] += idx;
            edgeConnections[Edge(indices[i+2], indices[i])]   += idx;
        }

        QHashIterator<Edge, QSet<uint32_t> > it(edgeConnections);

        while(it.hasNext())
        {
            it.next();
            const QSet<uint32_t>& nodes = it.value();

            if(nodes.size() == 2)
            {
                const uint32_t n1Idx = *nodes.begin();
                const uint32_t n2Idx = *(nodes.begin() + 1);

                assert(n1Idx < graph.getNumNodes());
                assert(n2Idx < graph.getNumNodes());

                const Pathfinding::NavigationGraph::node_type* n1 = graph.getNode(n1Idx);
                const Pathfinding::NavigationGraph::node_type* n2 = graph.getNode(n2Idx);
                const ailib::real_type distance = heuristic(*n1, *n2);

                graph.addEdge(n1Idx, n2Idx, distance);
                graph.addEdge(n2Idx, n1Idx, distance);
            }
        }
    }
    else
    {
        qWarning() << "No meshes were attached to the navmesh scene node. "
                   << "Returning an empty navgraph.";
    }

    mGraph = graph;
}

void Pathfinding::visualizeNavGraph(DebugDrawer* drawer) const
{
    const Pathfinding::NavigationGraph::node_type* const firstNode = mGraph.getNodesBegin();
    const Pathfinding::NavigationGraph::node_type* it = firstNode;
    for(; it != mGraph.getNodesEnd(); ++it)
    {
        drawer->drawCircle(it->getCentroid(), 0.1, 4, Ogre::ColourValue::Green, true);

        uint32_t currentIdx = it - firstNode;

        const ailib::Edge* it2 = mGraph.getSuccessorsBegin(currentIdx);
        const ailib::Edge* const end = mGraph.getSuccessorsEnd(currentIdx);
        for(; it2 != end; ++it2)
        {
            drawer->drawLine(it->getCentroid(),
                             mGraph.getNode(it2->targetIndex)->getCentroid(),
                             Ogre::ColourValue::Red);
        }
    }

    qDebug() << "Navigation mesh loaded with " << mGraph.getNumNodes() << " nodes.";
}

void Pathfinding::moveActor(Actor* actor, const Ogre::Vector3& target)
{
    bool isAlreadyThere;
    ailib::AStar<Pathfinding::NavigationGraph>::path_type path;
    path = Pathfinding::planPath(actor->getPosition(),
                                 target,
                                 &isAlreadyThere);

    qDebug() << "Path size is " << path.size() << "hops.";

    if(path.empty())
    {
        qWarning() << "Could not find a path for actor [ "
                   << actor->getName() << " ] to reach "
                   << Ogre::StringConverter::toString(target).c_str();
        return;
    }

    if(isAlreadyThere)
    {
        qDebug() << "Target triangle has already been reached.";
        actor->setKnowledge("movement_target", QVariant::fromValue(target));
        return;
    }

    QVector<Ogre::Vector3> qpath;
    // Don't save the first entry as we immediately store it in "movement_target"
    ailib::AStar<Pathfinding::NavigationGraph>::path_type::const_iterator it = path.begin() + 1;
    for(; it != path.end(); ++it)
    {
        qpath += (*it)->getCentroid();
    }

    actor->setKnowledge("current_path", QVariant::fromValue(qpath));
    actor->setKnowledge("movement_target", QVariant::fromValue(qpath.first()));
}
