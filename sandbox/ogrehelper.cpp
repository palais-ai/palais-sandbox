#include "ogrehelper.h"
#include "astar.h"

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

#include <MmOptimiseTool.h>
#include <MmOgreEnvironment.h>

using namespace meshmagick;

namespace Ogre
{
static uint qHash(const Ogre::Vector3& key)
{
     return static_cast<uint>(key.x) ^ static_cast<uint>(key.z) + static_cast<uint>(key.y);
}
};

namespace OgreHelper
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

static uint qHash(const Edge& key)
{
     return key.index1 ^ key.index2;
}

Triangle::Triangle(const Ogre::Vector3& v1,
                   const Ogre::Vector3& v2,
                   const Ogre::Vector3& v3)
{
    a = v1;
    b = v2;
    c = v3;
}

Ogre::Vector3 Triangle::getCentroid() const
{
    float lenA = c.distance(b), lenB = a.distance(c), lenC = b.distance(a);
    float mul = lenA*lenB*lenC;
    return fromBarycentric(mul, mul, mul);
}

Ogre::Vector3 Triangle::fromBarycentric(float x, float y, float z) const
{
    float div = x+y+z;
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

TriangleNode::TriangleNode(const Ogre::Vector3& v1,
                           const Ogre::Vector3& v2,
                           const Ogre::Vector3& v3) :
    Triangle(v1, v2, v3)
{
    ;
}

class DistanceComparator
{
public:
    DistanceComparator(const Ogre::Vector3& reference) :
        mReference(reference)
    {
        ;
    }

    bool operator() (const TriangleNode& lv, const TriangleNode& rv) const
    {
        return lv.getCentroid().squaredDistance(mReference) < rv.getCentroid().squaredDistance(mReference);
    }
private:
    const Ogre::Vector3& mReference;
};

const TriangleNode* getNavNodeClosestToPoint(const NavigationGraph& graph, const Ogre::Vector3& point)
{
    NavigationGraph::node_collection nodeCopy(graph.nodes.begin(), graph.nodes.end());

    DistanceComparator comp(point);
    std::sort(nodeCopy.begin(), nodeCopy.end(), comp);

    for(NavigationGraph::node_collection::const_iterator it = nodeCopy.begin(); it != nodeCopy.end(); ++it)
    {
        const NavigationGraph::node_type& node = *it;

        if(node.isProjectionInside(point))
        {
           return &node;
        }
    }

    return NULL;
}

OgreHelper::NavigationGraph makeNavGraphFromOgreNode(Ogre::SceneNode* node)
{
    OgreHelper::NavigationGraph graph;

    if(node->numAttachedObjects() > 0)
    {
        const Ogre::MeshPtr mesh = static_cast<Ogre::Entity*>(node->getAttachedObject(0))->getMesh();

        OgreEnvironment ogreEnv;
        ogreEnv.initialize(false, Ogre::LogManager::getSingleton().getDefaultLog());
        OptimiseTool optimiseTool;

        // Remove duplicate vertices using libmeshmagick
        // This is important for the following algorithm to generate a discrete navigation graph from
        // the continuous navigation mesh.
        optimiseTool.processMesh(mesh);

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

        QHash<Edge, QSet<TriangleNode*> > edgeConnections;

        graph.nodes.reserve(indexCount / 3);
        for(size_t i = 0; i < indexCount; i+=3)
        {
            graph.nodes.push_back(TriangleNode(vertices[indices[i]],
                                               vertices[indices[i+1]],
                                               vertices[indices[i+2]]));

            TriangleNode* node = &graph.nodes.back();

            edgeConnections[Edge(indices[i], indices[i+1])] += node;
            edgeConnections[Edge(indices[i+1], indices[i+2])] += node;
            edgeConnections[Edge(indices[i+2], indices[i])] += node;
        }

        QHashIterator<Edge, QSet<TriangleNode*> > it(edgeConnections);

        while(it.hasNext())
        {
            it.next();
            const QSet<TriangleNode*>& nodes = it.value();

            qDebug() << nodes.size();
            if(nodes.size() == 2)
            {
                TriangleNode* n1 = *nodes.begin();
                TriangleNode* n2 = *(nodes.begin() + 1);

                float distance = n1->getCentroid().distance(n2->getCentroid());
                n1->edges.push_back(ailib::Edge::makeEdge(n2, distance));
                n2->edges.push_back(ailib::Edge::makeEdge(n1, distance));
            }
        }
    }

    return graph;
}

// CREDITS: Public domain license, from http://www.ogre3d.org/tikiwiki/tiki-index.php?page=RetrieveVertexData
void getMeshInformation(const Ogre::MeshPtr mesh,
                        size_t &vertex_count,
                        Ogre::Vector3* &vertices,
                        size_t &index_count,
                        unsigned* &indices,
                        const Ogre::Vector3 &position,
                        const Ogre::Quaternion &orient,
                        const Ogre::Vector3 &scale)
{
    bool added_shared = false;
    size_t current_offset = 0;
    size_t shared_offset = 0;
    size_t next_offset = 0;
    size_t index_offset = 0;

    vertex_count = index_count = 0;

    // Calculate how many vertices and indices we're going to need
    for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        // We only need to add the shared vertices once
        if(submesh->useSharedVertices)
        {
            if( !added_shared )
            {
                vertex_count += mesh->sharedVertexData->vertexCount;
                added_shared = true;
            }
        }
        else
        {
            vertex_count += submesh->vertexData->vertexCount;
        }
        // Add the indices
        index_count += submesh->indexData->indexCount;
    }

    // Allocate space for the vertices and indices
    vertices = new Ogre::Vector3[vertex_count];
    indices = new unsigned[index_count];

    added_shared = false;

    // Run through the submeshes again, adding the data into the arrays
    for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);

        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

        if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
        {
            if(submesh->useSharedVertices)
            {
                added_shared = true;
                shared_offset = current_offset;
            }

            const Ogre::VertexElement* posElem =
                vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

            Ogre::HardwareVertexBufferSharedPtr vbuf =
                vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

            unsigned char* vertex =
                static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

            // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
            //  as second argument. So make it float, to avoid trouble when Ogre::Real will
            //  be comiled/typedefed as double:
            //Ogre::Real* pReal;
            float* pReal;

            for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
            {
                posElem->baseVertexPointerToElement(vertex, &pReal);
                Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
                vertices[current_offset + j] = (orient * (pt * scale)) + position;
            }

            vbuf->unlock();
            next_offset += vertex_data->vertexCount;
        }

        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

        unsigned long* pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

        size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;

        if ( use32bitindexes )
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = pLong[k] + static_cast<unsigned long>(offset);
            }
        }
        else
        {
            for ( size_t k = 0; k < numTris*3; ++k)
            {
                indices[index_offset++] = static_cast<unsigned long>(pShort[k]) +
                                          static_cast<unsigned long>(offset);
            }
        }

        ibuf->unlock();
        current_offset = next_offset;
    }
}
} // END NS OGREHELPER
