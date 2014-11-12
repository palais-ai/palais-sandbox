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

static const Ogre::Vector3 initialPosition(0, 0, 300);

extern QMutex g_engineMutex;

CameraNodeObject::CameraNodeObject(QObject *parent) :
    QObject(parent),
    OgreCameraWrapper(),
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
    camera->setAutoTracking(true, sceneManager->getRootSceneNode());

    m_camera = camera;
    m_node = sceneManager->getRootSceneNode()->createChildSceneNode();
    m_node->attachObject(camera);
    camera->move(initialPosition);

    Ogre::SceneNode::ChildNodeIterator children = sceneManager->getRootSceneNode()->getChildIterator();
    Ogre::AxisAlignedBox aabb;
    while (children.hasMoreElements())
    {
        Ogre::SceneNode* child = static_cast<Ogre::SceneNode*>(children.getNext());

        aabb.merge(child->_getWorldAABB());
        child->showBoundingBox(true);
    }

    g_engineMutex.unlock();
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
    m_camera->setPosition(initialPosition * (1 / m_zoom));
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
