/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "QOCamera.h"
#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreNode.h>
#include <OgreCamera.h>
#include <OgreAxisAlignedBox.h>
#include <OgreStringConverter.h>
#include <QDebug>
#include <QThread>
#include <cmath>

extern QThread* g_engineThread;

CameraHandler::CameraHandler() :
    mInitialDistance(0),
    mNode(NULL),
    mCamera(NULL)
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
    camera->setNearClipDistance(0.1);
    camera->setFarClipDistance(1000);
    camera->setAspectRatio(1);

    mCamera = camera;
    mNode = sceneManager->getRootSceneNode()->createChildSceneNode();
    mNode->attachObject(camera);
    mCamera->setPosition(0,0,0);

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

    const Ogre::Radian after(mCamera->getOrientation().getPitch() + Ogre::Degree(p));
    const Ogre::Radian lowerLimit(-Ogre::Math::PI/2 + 0.1);
    const Ogre::Radian upperLimit(Ogre::Math::PI/2 - 0.1);

    if(after < lowerLimit || after > upperLimit)
    {
        // Don't rotate to prevent gimbal lock.
    }
    else
    {
        mCamera->pitch(Ogre::Degree(p));
    }

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

    Ogre::Vector3 previousPosition = mCamera->getPosition();

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
    if(fabs(previousPosition.length()) <= 0.001) // Initial case
    {
        mCamera->moveRelative(Ogre::Vector3(0, 0, mInitialDistance));
    }
    else // Re-focus case
    {
        mCamera->setPosition(previousPosition);
    }

    emit zoomChanged(1);
    emit focusNodeChanged(node);
}

float CameraHandler::getDistanceToAutoTrackingTarget() const
{
    return (mCamera->getPosition()
            - mCamera->getAutoTrackTarget()->_getDerivedPosition()).length();;
}

QOCamera::QOCamera(QObject *parent) :
    QObject(parent),
    mZoom(0),
    mNode(NULL),
    mFocusNode(NULL),
    mCamera(NULL),
    mHandler(new CameraHandler)
{
    // First one instantiation has to be direct.
    connect(this, &QOCamera::requestCreateCameraWithCurrentSceneManager,
            mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager,
            Qt::DirectConnection);
    connect(this, &QOCamera::requestRelativeYawChanged,
            mHandler.data(), &CameraHandler::onRelativeYawChanged);
    connect(this, &QOCamera::requestRelativePitchChanged,
            mHandler.data(), &CameraHandler::onRelativePitchChanged);
    connect(this, &QOCamera::requestZoomChanged,
            mHandler.data(), &CameraHandler::onZoomChanged);
    connect(this, &QOCamera::requestFocusSceneNode,
            mHandler.data(), &CameraHandler::onFocusSceneNode);

    connect(mHandler.data(), &CameraHandler::zoomChanged,
            this, &QOCamera::onZoomChanged);
    connect(mHandler.data(), &CameraHandler::setupChanged,
            this, &QOCamera::onSetupChanged,
            Qt::DirectConnection);
    connect(mHandler.data(), &CameraHandler::focusNodeChanged,
            this, &QOCamera::onFocusNodeChanged);

    createCameraWithCurrentSceneManager();

    // The next one's should be auto-resolved.
    disconnect(this, &QOCamera::requestCreateCameraWithCurrentSceneManager,
               mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);
    connect(this, &QOCamera::requestCreateCameraWithCurrentSceneManager,
            mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);

    disconnect(mHandler.data(), &CameraHandler::setupChanged,
               this, &QOCamera::onSetupChanged);
    connect(mHandler.data(), &CameraHandler::setupChanged,
            this, &QOCamera::onSetupChanged);
}

void QOCamera::createCameraWithCurrentSceneManager()
{
    if(QThread::currentThread() == g_engineThread)
    {
        disconnect(this, &QOCamera::requestCreateCameraWithCurrentSceneManager,
                mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);
        // Ensure this signal is handled synchronously if called from the engine's thread.
        connect(this, &QOCamera::requestCreateCameraWithCurrentSceneManager,
                mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager,
                Qt::DirectConnection);

        disconnect(mHandler.data(), &CameraHandler::setupChanged,
                   this, &QOCamera::onSetupChanged);
        connect(mHandler.data(), &CameraHandler::setupChanged,
                this, &QOCamera::onSetupChanged,
                Qt::DirectConnection);
    }

    emit requestCreateCameraWithCurrentSceneManager();

    if(QThread::currentThread() == g_engineThread)
    {
        // The next one's should be auto-resolved.
        disconnect(this, &QOCamera::requestCreateCameraWithCurrentSceneManager,
                mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);
        connect(this, &QOCamera::requestCreateCameraWithCurrentSceneManager,
                mHandler.data(), &CameraHandler::onCreateCameraWithCurrentSceneManager);

        disconnect(mHandler.data(), &CameraHandler::setupChanged,
                   this, &QOCamera::onSetupChanged);
        connect(mHandler.data(), &CameraHandler::setupChanged,
                this, &QOCamera::onSetupChanged);
    }
}

void QOCamera::focus(Ogre::SceneNode* node)
{
    emit requestFocusSceneNode(node);
}

Ogre::SceneNode* QOCamera::sceneNode() const
{
    return mNode;
}

Ogre::SceneNode* QOCamera::focusedNode() const
{
    return mFocusNode;
}

Ogre::Camera* QOCamera::camera() const
{
    return mCamera;
}

void QOCamera::yaw(qreal y)
{
    emit requestRelativeYawChanged(y);
}

void QOCamera::pitch(qreal p)
{
    emit requestRelativePitchChanged(p);
}

void QOCamera::zoom(qreal z)
{
    emit requestZoomChanged(z);
}

qreal QOCamera::getZoom() const
{
    return mZoom;
}

void QOCamera::onZoomChanged(qreal zoom)
{
    mZoom = zoom;
}

void QOCamera::onFocusNodeChanged(Ogre::SceneNode* focusNode)
{
    mFocusNode = focusNode;
}

void QOCamera::onSetupChanged(Ogre::Camera* camera, Ogre::SceneNode* sceneNode)
{
    mCamera = camera;
    mNode = sceneNode;

    emit cameraChanged(camera);
}
