#ifndef OGREHELPER_H
#define OGREHELPER_H

#include <OgreVector3.h>
#include <OgreQuaternion.h>

namespace Ogre
{
    class Mesh;
}

namespace OgreHelper
{
const Ogre::MeshPtr optimizeMesh(const Ogre::MeshPtr mesh);

// CREDITS: Public domain license, from
// http://www.ogre3d.org/tikiwiki/tiki-index.php?page=RetrieveVertexData
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
