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
#include <QThread>

#include <cmath>

extern QThread* g_engineThread;

CameraHandler::CameraHandler() :
    mInitialDistance(0),
    mNode(0),
    mCamera(0)
{
    moveToThread(g_engineThread);
}

void CameraHandler::onCreateCameraWithCurrentSceneManager()
{
    static int sCameraCounter = 0;

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

    QString instanceName;
    instanceName.sprintf("camera_%d", sCameraCounter++);

    Ogre::Camera *camera = sceneManager->createCamera(instanceName.toLatin1().data());
    camera->setNearClipDistance(2);
    camera->setFarClipDistance(750);
    camera->setAspectRatio(1);

    mCamera = camera;
    mNode = sceneManager->getRootSceneNode()->createChildSceneNode();
    mNode->attachObject(camera);

    fitToContain(sceneManager->getRootSceneNode());

    emit setupChanged(mCamera, mNode);
}

void CameraHandler::onRelativeYawChanged(qreal y)
{
    Ogre::Real dist = getDistanceToAutoTrackingTarget();

    mCamera->setPosition(mCamera->getAutoTrackTarget()->_getDerivedPosition());
    mCamera->yaw(Ogre::Degree(y));
    mCamera->moveRelative(Ogre::Vector3(0, 0, dist));
}

void CameraHandler::onRelativePitchChanged(qreal p)
{
    Ogre::Real dist = getDistanceToAutoTrackingTarget();

    mCamera->setPosition(mCamera->getAutoTrackTarget()->_getDerivedPosition());
    mCamera->pitch(Ogre::Degree(p));
    mCamera->moveRelative(Ogre::Vector3(0, 0, dist));
}

void CameraHandler::onZoomChanged(qreal z)
{
    mCamera->setPosition(mCamera->getAutoTrackTarget()->_getDerivedPosition());
    mCamera->moveRelative(Ogre::Vector3(0, 0, z * mInitialDistance));

    emit zoomChanged(z);
}

void CameraHandler::onFocusSceneNode(Ogre::SceneNode* node)
{
    fitToContain(node);
}

void CameraHandler::fitToContain(Ogre::SceneNode* node)
{
    if(!node)
    {
        return;
    }

    mCamera->setAutoTracking(true, node);
    mCamera->setFixedYawAxis(true);
    mCamera->setPosition(node->_getDerivedPosition());
    mCamera->setOrientation(node->_getDerivedOrientation());
    mCamera->yaw(Ogre::Degree(45));
    mCamera->pitch(-Ogre::Degree(45));

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

    if(mInitialDistance == 0)
    {
        // Scale view to fit
        mInitialDistance = ((boundingRadius) /
                            tan(mCamera->getFOVy().valueRadians() / 2.f));
    }

    // Reset zoom level
    mCamera->moveRelative(Ogre::Vector3(0, 0, mInitialDistance));

    emit zoomChanged(1);
}

float CameraHandler::getDistanceToAutoTrackingTarget() const
{
    return (mCamera->getPosition()
            - mCamera->getAutoTrackTarget()->_getDerivedPosition()).length();;
}

CameraNodeObject::CameraNodeObject(QObject *parent) :
    QObject(parent),
    mZoom(0),
    mNode(0),
    mCamera(0),
    mHandler(new CameraHandler)
{
    // First one instantiation has to be direct.
    connect(this, &CameraNodeObject::requestCreateCameraWithCurrentSceneManager,
            mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager,
            Qt::DirectConnection);
    connect(this, &CameraNodeObject::requestRelativeYawChanged,
            mHandler.data(), &CameraHandler::onRelativeYawChanged);
    connect(this, &CameraNodeObject::requestRelativePitchChanged,
            mHandler.data(), &CameraHandler::onRelativePitchChanged);
    connect(this, &CameraNodeObject::requestZoomChanged,
            mHandler.data(), &CameraHandler::onZoomChanged);
    connect(this, &CameraNodeObject::requestFocusSceneNode,
            mHandler.data(), &CameraHandler::onFocusSceneNode);

    connect(mHandler.data(), &CameraHandler::zoomChanged,
            this, &CameraNodeObject::onZoomChanged);
    connect(mHandler.data(), &CameraHandler::setupChanged,
            this, &CameraNodeObject::onSetupChanged,
            Qt::DirectConnection);

    createCameraWithCurrentSceneManager();

    // The next one's should be auto-resolved.
    disconnect(this, &CameraNodeObject::requestCreateCameraWithCurrentSceneManager,
               mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);
    connect(this, &CameraNodeObject::requestCreateCameraWithCurrentSceneManager,
            mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);

    disconnect(mHandler.data(), &CameraHandler::setupChanged,
               this, &CameraNodeObject::onSetupChanged);
    connect(mHandler.data(), &CameraHandler::setupChanged,
            this, &CameraNodeObject::onSetupChanged);
}

void CameraNodeObject::createCameraWithCurrentSceneManager()
{
    if(QThread::currentThread() == g_engineThread)
    {
        disconnect(this, &CameraNodeObject::requestCreateCameraWithCurrentSceneManager,
                mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);
        // Ensure this signal is handled synchronously if called from the engine's thread.
        connect(this, &CameraNodeObject::requestCreateCameraWithCurrentSceneManager,
                mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager,
                Qt::DirectConnection);

        disconnect(mHandler.data(), &CameraHandler::setupChanged,
                   this, &CameraNodeObject::onSetupChanged);
        connect(mHandler.data(), &CameraHandler::setupChanged,
                this, &CameraNodeObject::onSetupChanged,
                Qt::DirectConnection);
    }

    emit requestCreateCameraWithCurrentSceneManager();

    if(QThread::currentThread() == g_engineThread)
    {
        // The next one's should be auto-resolved.
        disconnect(this, &CameraNodeObject::requestCreateCameraWithCurrentSceneManager,
                mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);
        connect(this, &CameraNodeObject::requestCreateCameraWithCurrentSceneManager,
                mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);

        disconnect(mHandler.data(), &CameraHandler::setupChanged,
                   this, &CameraNodeObject::onSetupChanged);
        connect(mHandler.data(), &CameraHandler::setupChanged,
                this, &CameraNodeObject::onSetupChanged);
    }
}

void CameraNodeObject::focus(Ogre::SceneNode* node)
{
    emit requestFocusSceneNode(node);
}

Ogre::SceneNode* CameraNodeObject::sceneNode() const
{
    return mNode;
}

Ogre::Camera* CameraNodeObject::camera() const
{
    return mCamera;
}

void CameraNodeObject::yaw(qreal y)
{
    emit requestRelativeYawChanged(y);
}

void CameraNodeObject::pitch(qreal p)
{
    emit requestRelativePitchChanged(p);
}

void CameraNodeObject::zoom(qreal z)
{
    emit requestZoomChanged(z);
}

qreal CameraNodeObject::getZoom() const
{
    return mZoom;
}

void CameraNodeObject::onZoomChanged(qreal zoom)
{
    mZoom = zoom;
}

void CameraNodeObject::onSetupChanged(Ogre::Camera* camera, Ogre::SceneNode* sceneNode)
{
    mCamera = camera;
    mNode = sceneNode;

    emit cameraChanged(camera);
}
