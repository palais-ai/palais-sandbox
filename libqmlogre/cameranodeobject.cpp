/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "cameranodeobject.h"

#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreNode.h>
#include <OgreCamera.h>
#include <OgreAxisAlignedBox.h>

#include <QDebug>
#include <cmath>

extern QMutex g_engineMutex;

CameraNodeObject::CameraNodeObject(QObject *parent) :
    QObject(parent),
    OgreCameraWrapper(),
    mInitialPosition(0,0,0),
    m_camera(0),
    m_yaw(0),
    m_pitch(0),
    m_zoom(1)
{
    Ogre::SceneManager *sceneManager = Ogre::Root::getSingleton().getSceneManager("mySceneManager");

    // let's use the current memory address to create a unique name
    QString instanceName;
    instanceName.sprintf("camera_%08p", this);

    g_engineMutex.lock();
    Ogre::Camera *camera = sceneManager->createCamera(instanceName.toLatin1().data());
    camera->setNearClipDistance(1);
    camera->setFarClipDistance(99999);
    camera->setAspectRatio(1);

    m_camera = camera;
    m_node = sceneManager->getRootSceneNode()->createChildSceneNode();
    m_node->attachObject(camera);

    fitToContain(sceneManager->getRootSceneNode());

    g_engineMutex.unlock();
}

void CameraNodeObject::fitToContain(Ogre::SceneNode* node)
{
    if(!node)
    {
        return;
    }

    m_camera->setAutoTracking(true, node);

    Ogre::SceneNode::ChildNodeIterator children = node->getChildIterator();
    Ogre::AxisAlignedBox aabb;

    // Compute the bounding radius of a sphere at origin containing all child objects
    Ogre::Real boundingRadius = 0;
    while (children.hasMoreElements())
    {
        Ogre::SceneNode* child = static_cast<Ogre::SceneNode*>(children.getNext());
        bool allVisible = true;

        Ogre::SceneNode::ObjectIterator objects = child->getAttachedObjectIterator();

        while (objects.hasMoreElements())
        {
            Ogre::MovableObject* object = static_cast<Ogre::MovableObject*>(objects.getNext());
            if(object->getVisible())
            {
                aabb.merge(object->getWorldBoundingBox());
                const Ogre::Real radius = object->getBoundingRadius();
                if(radius > boundingRadius)
                {
                    boundingRadius = radius;
                }
            }
            else
            {
                allVisible = false;
            }
        }

        if(allVisible)
        {
            //child->showBoundingBox(true);
        }
    }

    // Scale view to fit
    mInitialPosition = Ogre::Vector3(1, 1, 0);
    mInitialPosition = mInitialPosition.normalise() * ((boundingRadius / 2.f) / tan(m_camera->getFOVy().valueRadians() / 2.f));

    // Reset zoom level
    m_zoom = 1;
    m_camera->move(mInitialPosition);
}

void CameraNodeObject::updateRotation()
{
    g_engineMutex.lock();
    m_node->resetOrientation();
    m_node->yaw(Ogre::Radian(Ogre::Degree(m_yaw)));
    m_node->pitch(Ogre::Radian(Ogre::Degree(m_pitch)));
    g_engineMutex.unlock();
}

void CameraNodeObject::setZoom(qreal z)
{
    g_engineMutex.lock();
    m_zoom = z;
    m_node->resetOrientation();
    m_camera->setPosition(mInitialPosition * (1 / m_zoom));
    g_engineMutex.unlock();
    updateRotation();
}

void CameraNodeObject::setWireframeMode(bool enabled)
{
    if(m_camera)
    {
        m_camera->setPolygonMode(enabled ? Ogre::PM_WIREFRAME : Ogre::PM_SOLID);
    }
}

bool CameraNodeObject::getWireframeMode() const
{
    if(m_camera)
    {
        return m_camera->getPolygonMode() == Ogre::PM_WIREFRAME;
    }
    else
    {
        return false;
    }
}
