#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "AStar.h"

#include <QObject>
#include <QScriptValue>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

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

class Pathfinding : public QObject
{
    Q_OBJECT
public:
    typedef ailib::Graph<Triangle, 3> NavigationGraph;

    void update(Scene& scene, float deltaTime);
    void updateActor(Actor& actor, float deltaTime);

    void initNavGraphFromOgreNode(Ogre::SceneNode* node);
    void visualizeNavGraph(DebugDrawer* drawer) const;
    Q_INVOKABLE void moveActor(Actor* actor,
                               const Ogre::Vector3& target,
                               QScriptValue onFinishedCallback =  QScriptValue::UndefinedValue);

    ailib::AStar<NavigationGraph>::path_type planPath(const Ogre::Vector3& from,
                                                      const Ogre::Vector3& to,
                                                      bool* isAlreadyThere = NULL);

    const NavigationGraph::node_type* getNavNodeClosestToPoint(const Ogre::Vector3& point);
private slots:
    void onActorDestroyed(QObject* actor);
private:
    NavigationGraph mGraph;
};

#endif // PATHFINDING_H
