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
#include <QMutexLocker>

#include <cmath>

extern QMutex g_engineMutex;

CameraNodeObject::CameraNodeObject(QObject *parent) :
    QObject(parent),
    mInitialPosition(0,0,0),
    m_node(0),
    m_camera(0),
    m_yaw(0),
    m_pitch(0),
    m_zoom(1)
{
    QMutexLocker locker(&g_engineMutex);

    createCameraWithCurrentSceneManager();
}

void CameraNodeObject::createCameraWithCurrentSceneManager()
{
    Ogre::Root* root = Ogre::Root::getSingletonPtr();
    if(!root)
    {
        qFatal("We need an ogre root to create a camera with.");
        return;
    }

    if(!root->hasSceneManager("TheSceneManager"))
    {
        qFatal("We need a scene manager to create a camera with.");
        return;
    }

    Ogre::SceneManager *sceneManager = root->getSceneManager("TheSceneManager");

    // let's use the current memory address to create a unique name
    QString instanceName;
    instanceName.sprintf("camera_%08p", this);

    Ogre::Camera *camera = sceneManager->createCamera(instanceName.toLatin1().data());
    camera->setNearClipDistance(1);
    camera->setFarClipDistance(99999);
    camera->setAspectRatio(1);

    m_camera = camera;
    m_node = sceneManager->getRootSceneNode()->createChildSceneNode();
    m_node->attachObject(camera);

    fitToContain(sceneManager->getRootSceneNode());

    emit cameraChanged(m_camera);
}

void CameraNodeObject::focus(Ogre::SceneNode* node)
{
    QMutexLocker locker(&g_engineMutex);

    fitToContain(node);
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
    }

    // Scale view to fit
    mInitialPosition = Ogre::Vector3(1, 1, 0);
    mInitialPosition = mInitialPosition.normalise()
                       * ((boundingRadius / 2.f) /
                          tan(m_camera->getFOVy().valueRadians() / 2.f));

    // Reset zoom level
    m_zoom = 1;
    m_camera->move(mInitialPosition);
}

Ogre::SceneNode* CameraNodeObject::sceneNode() const
{
    return m_node;
}

Ogre::Camera* CameraNodeObject::camera() const
{
    return m_camera;
}

qreal CameraNodeObject::yaw() const
{
    QMutexLocker locker(&g_engineMutex);

    return m_yaw;
}

qreal CameraNodeObject::pitch() const
{
    QMutexLocker locker(&g_engineMutex);

    return m_pitch;
}

qreal CameraNodeObject::zoom() const
{
    QMutexLocker locker(&g_engineMutex);

    return m_zoom;
}

void CameraNodeObject::setYaw(qreal y)
{
    QMutexLocker locker(&g_engineMutex);

    m_yaw = y;
    updateRotation();
}

void CameraNodeObject::setPitch(qreal p)
{
    QMutexLocker locker(&g_engineMutex);

    m_pitch = p;
    updateRotation();
}

void CameraNodeObject::updateRotation()
{
    m_node->resetOrientation();
    m_node->yaw(Ogre::Radian(Ogre::Degree(m_yaw)));
    m_node->pitch(Ogre::Radian(Ogre::Degree(m_pitch)));
}

void CameraNodeObject::setZoom(qreal z)
{
    QMutexLocker locker(&g_engineMutex);

    m_zoom = z;
    m_node->resetOrientation();
    m_camera->setPosition(mInitialPosition * (1 / m_zoom));

    updateRotation();
}

void CameraNodeObject::setWireframeMode(bool enabled)
{
    QMutexLocker locker(&g_engineMutex);

    if(m_camera)
    {
        m_camera->setPolygonMode(enabled ? Ogre::PM_WIREFRAME : Ogre::PM_SOLID);
    }
}

bool CameraNodeObject::getWireframeMode() const
{
    QMutexLocker locker(&g_engineMutex);

    if(m_camera)
    {
        return m_camera->getPolygonMode() == Ogre::PM_WIREFRAME;
    }
    else
    {
        return false;
    }
}
