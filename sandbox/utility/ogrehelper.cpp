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
#include <OgreStringConverter.h>

#include <MmOptimiseTool.h>
#include <MmOgreEnvironment.h>

using namespace meshmagick;

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

const NavigationGraph::node_type* getNavNodeClosestToPoint(const NavigationGraph& graph, const Ogre::Vector3& point)
{
    for(const NavigationGraph::node_type* it = graph.getNodesBegin(); it != graph.getNodesEnd(); ++it)
    {
        const NavigationGraph::node_type& node = *it;

        if(node.isProjectionInside(point))
        {
           return &node;
        }
    }

    return NULL;
}

static ailib::real_type euclideanHeuristic(const OgreHelper::NavigationGraph::node_type& n1,
                                           const OgreHelper::NavigationGraph::node_type& n2)
{
    return n1.getCentroid().squaredDistance(n2.getCentroid());
}

ailib::AStar<NavigationGraph>::path_type planPath(const NavigationGraph& navGraph,
                                                  const Ogre::Vector3& from,
                                                  const Ogre::Vector3& to,
                                                  bool* isAlreadyThere)
{
    const NavigationGraph::node_type* start = getNavNodeClosestToPoint(navGraph, from);
    if(!start)
    {
        qWarning() << "Target position "
                   << Ogre::StringConverter::toString(from).c_str()
                   << " is not covered by the navmesh. No valid path could be calculated.";
        return ailib::AStar<NavigationGraph>::path_type();
    }

    const NavigationGraph::node_type* goal = getNavNodeClosestToPoint(navGraph, to);
    if(!goal)
    {
        qWarning() << "Target position "
                   << Ogre::StringConverter::toString(to).c_str()
                   << " is not covered by the navmesh. No valid path could be calculated.";
        return ailib::AStar<NavigationGraph>::path_type();
    }

    ailib::AStar<NavigationGraph> astar(navGraph);
    ailib::AStar<NavigationGraph>::path_type path = astar.findPath(start, goal, euclideanHeuristic);

    if(isAlreadyThere)
    {
        *isAlreadyThere = path.size() == 1 && path[0] == start;
    }

    return path;
}

static const Ogre::MeshPtr optimizeMesh(const Ogre::MeshPtr mesh)
{
    if(mesh.get())
    {
        OgreEnvironment ogreEnv;
        ogreEnv.initialize(false, Ogre::LogManager::getSingleton().getDefaultLog());
        OptimiseTool optimiseTool;

        optimiseTool.setPosTolerance(1e-06);
        optimiseTool.setNormTolerance(1e-06);
        optimiseTool.setUVTolerance(1e-06);

        // Remove duplicate vertices using libmeshmagick
        // This is important for the following algorithm to generate a discrete navigation graph from
        // the continuous navigation mesh.
        optimiseTool.processMesh(mesh);
    }
    else
    {
        qWarning("MeshPtr mustn't be null. Can't optimize an uninitialized Mesh.");
    }

    return mesh;
}

OgreHelper::NavigationGraph makeNavGraphFromOgreNode(Ogre::SceneNode* node,
                                                     ailib::AStar<OgreHelper::NavigationGraph>::Heuristic heuristic)
{
    OgreHelper::NavigationGraph graph;

    if(node->numAttachedObjects() > 0)
    {
        const Ogre::MeshPtr mesh = optimizeMesh(static_cast<Ogre::Entity*>(node->getAttachedObject(0))->getMesh());

        if(!mesh.get())
        {
            qWarning() << "Optimized mesh ptr was null. Returning empty nav graph.";
            return graph;
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

        OgreHelper::NavigationGraph graph;
        QHash<Edge, QSet<uint32_t> > edgeConnections;
        for(size_t i = 0; i < indexCount; i+=3)
        {
            uint32_t idx = graph.addNode(OgreHelper::NavigationGraph::node_type(vertices[indices[i]],
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

                const OgreHelper::NavigationGraph::node_type* n1 = graph.getNode(n1Idx);
                const OgreHelper::NavigationGraph::node_type* n2 = graph.getNode(n2Idx);
                const ailib::real_type distance = heuristic(*n1, *n2);

                graph.addEdge(n1Idx, n2Idx, distance);
                graph.addEdge(n2Idx, n1Idx, distance);
            }
        }
        return graph;
    }
    else
    {
        qWarning() << "No meshes were attached to the navmesh scene node. Returning an empty navgraph.";
        return graph;
    }
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
        if(numTris > 0)
        {
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
        }
        current_offset = next_offset;
    }
}
} // END NS OGREHELPER
