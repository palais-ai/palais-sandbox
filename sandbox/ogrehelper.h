#ifndef OGREHELPER_H
#define OGREHELPER_H

#include "graph.h"

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


    Ogre::Vector3 a, b, c;

    Ogre::Vector3 getCentroid() const;
    Ogre::Vector3 fromBarycentric(float x, float y, float z) const;
    bool isProjectionInside(const Ogre::Vector3& point) const;
};

class TriangleNode : public Triangle, public ailib::BaseNode
{
public:
    TriangleNode(const Ogre::Vector3& v1,
                 const Ogre::Vector3& v2,
                 const Ogre::Vector3& v3);
};

typedef ailib::Graph<TriangleNode> NavigationGraph;

NavigationGraph makeNavGraphFromOgreNode(Ogre::SceneNode* node);
const TriangleNode* getNavNodeClosestToPoint(const NavigationGraph& graph, const Ogre::Vector3& point);

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
