#include "Pathfinding.h"
#include "IDAStar.h"
#include "Scheduler.h"
#include "OgreHelper.h"
#include "DebugDrawer.h"
#include "Bindings/JavascriptBindings.h"
#include "utility/MetatypeDeclarations.h"
#include "Actor.h"
#include "Scene.h"
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
#include <algorithm>

using namespace ailib;

Q_DECLARE_METATYPE(QScriptValue)
Q_DECLARE_METATYPE(Scheduler*)

extern Scene* gCurrentScene;

static ailib::real_type euclideanHeuristic(const Pathfinding::NavigationGraph::node_type& n1,
                                           const Pathfinding::NavigationGraph::node_type& n2)
{
    return n1.getCentroid().distance(n2.getCentroid());
}

PathfindingRequest::PathfindingRequest(Actor* actor,
                                      const Ogre::Vector3& target,
                                      QScriptValue onFinishedCallback) :
    actor(actor),
    target(target),
    onFinishedCallback(onFinishedCallback)
{
    ;
}

void Pathfinding::updateActor(Actor &actor, float deltaTime)
{
    static const float defaultActorSpeed = 0.5; // meters per second
    if(actor.hasKnowledge("movement_target"))
    {
        float actorSpeed;
        if(actor.hasKnowledge("movement_speed"))
        {
            bool ok;
            actorSpeed = actor.getKnowledge("movement_speed").toFloat(&ok);
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
        const float distanceToTarget = current.distance(target);
        if(distanceToTarget < 0.01)
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
                if(actor.hasKnowledge("goal_reached_callback"))
                {
                    QVariant variant = actor.getKnowledge("goal_reached_callback");
                    QScriptValue val = variant.value<QScriptValue>();
                    if(val.isFunction() && val.engine())
                    {
                        val.call();
                        JavaScriptBindings::checkScriptEngineException(*val.engine(),
                                                                       "Pathfinding."
                                                                       "GoalReachedCallback");
                    }
                    else
                    {
                        qWarning("[PATHFINDING] 'goal_reached_callback' should be a function.");
                    }
                }

                removeCallback(&actor);
                actor.removeKnowledge("movement_target");
                return;
            }
        }

        const float stepDistance = std::min(distanceToTarget, actorSpeed * deltaTime);
        Ogre::Vector3 step = (target - current).normalisedCopy() * stepDistance;
        actor.setPosition(current + step);
    }

    if(actor.hasKnowledge("lookat_target") &&
       actor.getKnowledge("lookat_target").canConvert<Ogre::Vector3>())
    {
        Ogre::Quaternion before = actor.getRotation();
        actor.lookAt(actor.getKnowledge("lookat_target").value<Ogre::Vector3>());
        Ogre::Quaternion after  = actor.getRotation();
        after.FromAngleAxis(after.getYaw(), Ogre::Vector3::UNIT_Y); // Lock pitch / roll.

        float rotationSpeed = 140; // Degrees per second

        if(actor.hasKnowledge("rotation_speed")  &&
           actor.getKnowledge("rotation_speed").canConvert<float>())
        {
            rotationSpeed = actor.getKnowledge("rotation_speed").value<float>();
        }

        Ogre::Quaternion delta = after * before.Inverse();
        delta.normalise();
        Ogre::Vector3 axis;
        Ogre::Degree angle;
        delta.ToAngleAxis(angle, axis);

        const float t = std::min((rotationSpeed * deltaTime) / angle.valueDegrees(), 1.f);
        Ogre::Quaternion between = Ogre::Quaternion::Slerp(t, before, after, true);
        between.normalise();
        actor.setRotation(between);
    }
}

void Pathfinding::update(Scene& scene, float deltaTime)
{
    QListIterator<QObject*> it(scene.getActorsArray());
    while(it.hasNext())
    {
        Actor* actor = qobject_cast<Actor*>(it.next());
        updateActor(*actor, deltaTime);
    }
}

namespace Local
{
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

static uint qHash(const Local::Edge& key)
{
     return key.index1 ^ key.index2;
}
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

// CREDITS:
// http://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c).
static void barycentric(Ogre::Vector3 p, Ogre::Vector3 a, Ogre::Vector3 b, Ogre::Vector3 c,
                        float &u, float &v, float &w)
{
    Ogre::Vector3 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = v0.dotProduct(v0);
    float d01 = v0.dotProduct(v1);
    float d11 = v1.dotProduct(v1);
    float d20 = v2.dotProduct(v0);
    float d21 = v2.dotProduct(v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}

bool Triangle::isProjectionInside(const Ogre::Vector3& point) const
{
    Ogre::Plane triPlane(a, b, c);
    Ogre::Vector3 pProjected = point - triPlane.getDistance(point)*triPlane.normal;

    float u, v , w;
    barycentric(pProjected, a, b, c, u, v, w);
    return (v > 0 || fabs(v) < 0.0001) &&
           (w > 0 || fabs(w) < 0.0001) &&
           (u > 0 || fabs(u) < 0.0001);
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

bool Pathfinding::planPath(const Ogre::Vector3& to,
                           Actor* actor,
                           QScriptValue onFinishedCallback)
{
    const Ogre::Vector3& from = actor->getPosition();
    const NavigationGraph::node_type* start = getNavNodeClosestToPoint(from);
    if(!start)
    {
        qWarning() << "Pathfinding.planPath: Start position "
                   << Ogre::StringConverter::toString(from).c_str()
                   << " is not covered by the navmesh. No valid path could be calculated.";
        return false;
    }

    const NavigationGraph::node_type* goal = getNavNodeClosestToPoint(to);
    if(!goal)
    {
        qWarning() << "Pathfinding.planPath: Target position "
                   << Ogre::StringConverter::toString(to).c_str()
                   << " is not covered by the navmesh. No valid path could be calculated.";
        return false;
    }

    if(gCurrentScene)
    {
        QScriptEngine& engine = gCurrentScene->getScriptEngine();
        QScriptValue value = engine.globalObject().property("Scheduler");
        ailib::Scheduler* s = qscriptvalue_cast<Scheduler*>(value);
        if(s)
        {
            ailib::AStarTask<NavigationGraph>* task = new ailib::AStarTask<NavigationGraph>
                                                                              (this,
                                                                               mGraph,
                                                                               start,
                                                                               goal,
                                                                               euclideanHeuristic);
            s->enqueue(task);
            mRequests.insert(task, PathfindingRequest(actor, to, onFinishedCallback));
            return true;
        }
    }

    ailib::AStar<NavigationGraph> astar(mGraph);
    ailib::AStar<NavigationGraph>::path_type path = astar.findPath(start,
                                                                   *goal,
                                                                   euclideanHeuristic);

    mRequests.insert(NULL, PathfindingRequest(actor, to, onFinishedCallback));
    onAStarResult(NULL, path, NULL);
    return true;
}

void Pathfinding::onAStarResult(AStarType* task,
                               const AStarType::path_type& path,
                               const connections_type* connections)
{
    Q_UNUSED(task);
    Q_UNUSED(connections);

    QHash<ailib::AStarTask<NavigationGraph>*, PathfindingRequest>::iterator rit =
            mRequests.find(static_cast<ailib::AStarTask<NavigationGraph>*>(task));
    if(rit == mRequests.end())
    {
        qWarning() << "Pathfinding.moveActor: Request was not added.";
        return;
    }

    //delete task;
    Actor* actor = rit->actor;
    Ogre::Vector3 target = rit->target;
    QScriptValue onFinishedCallback = rit->onFinishedCallback;

    mRequests.remove(static_cast<ailib::AStarTask<NavigationGraph>*>(task));

    if(!actor)
    {
        qWarning() << "Pathfinding.moveActor: Actor has not been set.";
        return;
    }

    if(path.empty())
    {
        qWarning() << "Pathfinding.moveActor: Could not find a path for actor [ "
                   << actor->getName() << " ] to reach "
                   << Ogre::StringConverter::toString(target).c_str();
        return;
    }

    const Ogre::Vector3& from = actor->getPosition();
    const NavigationGraph::node_type* start = getNavNodeClosestToPoint(from);
    bool isAlreadyThere = path.size() == 1 && path[0] == start;
    if(isAlreadyThere)
    {
        qDebug() << "Pathfinding.moveActor: Target triangle has already been reached.";
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

    // Add the move from the navigation mesh node to the actual goal aswell.
    qpath += target;

    actor->setKnowledge("current_path", QVariant::fromValue(qpath));
    actor->setKnowledge("movement_target", QVariant::fromValue(qpath.first()));

    // Residual from another pathfinding call, that was interrupted before reaching the goal.
    removeCallback(actor);

    if(!onFinishedCallback.isUndefined())
    {
        connect(actor, &Actor::removedFromScene,
                this, &Pathfinding::onActorRemoved);
        actor->setKnowledge("goal_reached_callback", QVariant::fromValue(onFinishedCallback));
    }
}

void
Pathfinding::initNavGraphFromNode(Ogre::SceneNode* node)
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
            qWarning("Pathfinding.initNavGraphFromNode: Optimized mesh ptr was null. "
                     "Returning empty nav graph.");
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

        QHash<Local::Edge, QSet<uint32_t> > edgeConnections;
        for(size_t i = 0; i < indexCount; i+=3)
        {
            uint32_t idx = graph.addNode(Pathfinding::NavigationGraph::node_type(vertices[indices[i]],
                                                                                 vertices[indices[i+1]],
                                                                                 vertices[indices[i+2]]));

            edgeConnections[Local::Edge(indices[i],   indices[i+1])] += idx;
            edgeConnections[Local::Edge(indices[i+1], indices[i+2])] += idx;
            edgeConnections[Local::Edge(indices[i+2], indices[i])]   += idx;
        }

        QHashIterator<Local::Edge, QSet<uint32_t> > it(edgeConnections);

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
        qWarning("Pathfinding.initNavGraphFromNode: No meshes were attached to the navmesh scene "
                 "node. Returning an empty navgraph.");
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

    qDebug() << "Pathfinding.visualizeNavGraph: Navigation mesh loaded with "
             << mGraph.getNumNodes() << " nodes.";
}

bool Pathfinding::moveActor(Actor* actor,
                            const Ogre::Vector3& target,
                            QScriptValue onFinishedCallback)
{
    return Pathfinding::planPath(target, actor, onFinishedCallback);
}

void Pathfinding::cancelMove(Actor* actor)
{
    actor->removeKnowledge("current_path");
    actor->removeKnowledge("movement_target");
    actor->removeKnowledge("goal_reached_callback");
}

void Pathfinding::onActorRemoved(Actor* actor)
{
    removeCallback(actor);
}

void Pathfinding::removeCallback(Actor* actor)
{
    if(actor->hasKnowledge("goal_reached_callback"))
    {
        disconnect(actor, &Actor::removedFromScene,
                   this, &Pathfinding::onActorRemoved);
        actor->removeKnowledge("goal_reached_callback");
    }
}
