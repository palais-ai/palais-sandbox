#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "AStarTask.h"
#include <QObject>
#include <QScriptValue>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <QHash>

class DebugDrawer;
class Actor;
class Scene;

namespace Ogre
{
    class SceneNode;
    class Mesh;
}

class Triangle
{
public:
    Triangle(const Ogre::Vector3& v1,
             const Ogre::Vector3& v2,
             const Ogre::Vector3& v3);

    Ogre::Vector3 a, b, c, centroid;

    const Ogre::Vector3& getCentroid() const;
    Ogre::Vector3 fromBarycentric(float x, float y, float z) const;
    bool isProjectionInside(const Ogre::Vector3& point) const;
    FORCE_INLINE bool operator==(const Triangle& other) const
    {
        // Using pointer comparison because polygons in the navmesh are unique.
        return this == &other;
    }
private:
    Ogre::Vector3 computeCentroid();
};

class PathfindingRequest
{
public:
    PathfindingRequest(Actor* actor,
                       const Ogre::Vector3& target,
                       QScriptValue onFinishedCallback);

    Actor* actor;
    Ogre::Vector3 target;
    QScriptValue onFinishedCallback;
};

class Pathfinding : public QObject, public ailib::AStarTaskListener<ailib::Graph<Triangle, 3> >
{
    Q_OBJECT
public:
    typedef ailib::Graph<Triangle, 3> NavigationGraph;

    void update(Scene& scene, float deltaTime);
    void updateActor(Actor& actor, float deltaTime);

    virtual void onAStarResult(AStarType* task,
                               const AStarType::path_type& path,
                               const connections_type* connections);

    void initNavGraphFromNode(Ogre::SceneNode* node);
    void visualizeNavGraph(DebugDrawer* drawer) const;
    Q_INVOKABLE bool moveActor(Actor* actor,
                               const Ogre::Vector3& target,
                               QScriptValue onFinishedCallback =  QScriptValue::UndefinedValue);

    Q_INVOKABLE void cancelMove(Actor* actor);

    bool planPath(const Ogre::Vector3& to,
                  Actor* actor,
                  QScriptValue onFinishedCallback);
    const NavigationGraph::node_type* getNavNodeClosestToPoint(const Ogre::Vector3& point);
private slots:
    void onActorRemoved(Actor* actor);
private:
    void removeCallback(Actor* actor);

    NavigationGraph mGraph;
    QHash<ailib::AStarTask<NavigationGraph>*, PathfindingRequest> mRequests;
};

#endif // PATHFINDING_H
