#ifndef OGREHELPER_H
#define OGREHELPER_H

#include "astar.h"

#include <QVector>

#include <OgreVector3.h>
#include <OgreQuaternion.h>

namespace Ogre
{
    class SceneNode;
    class Mesh;
}

namespace OgreHelper
{

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
private:
    Ogre::Vector3 computeCentroid();
};

typedef ailib::Graph<Triangle, 3> NavigationGraph;

NavigationGraph makeNavGraphFromOgreNode(Ogre::SceneNode* node,
                                         ailib::AStar<NavigationGraph>::Heuristic heuristic);

const NavigationGraph::node_type* getNavNodeClosestToPoint(const NavigationGraph& graph,
                                                           const Ogre::Vector3& point);

ailib::AStar<NavigationGraph>::path_type planPath(const NavigationGraph& navGraph,
                                                  const Ogre::Vector3& from,
                                                  const Ogre::Vector3& to,
                                                  bool* isAlreadyThere = NULL);

// CREDITS: Public domain license, from http://www.ogre3d.org/tikiwiki/tiki-index.php?page=RetrieveVertexData
void getMeshInformation(const Ogre::MeshPtr mesh,
                        size_t &vertex_count,
                        Ogre::Vector3* &vertices,
                        size_t &index_count,
                        unsigned* &indices,
                        const Ogre::Vector3 &position = Ogre::Vector3::ZERO,
                        const Ogre::Quaternion &orient = Ogre::Quaternion::IDENTITY,
                        const Ogre::Vector3 &scale = Ogre::Vector3::UNIT_SCALE);
};

#endif // OGREHELPER_H
