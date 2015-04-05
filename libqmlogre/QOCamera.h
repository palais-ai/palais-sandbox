/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#ifndef QOCamera_H
#define QOCamera_H

#include "qmlogre_global.h"
#include <QObject>
#include <QScopedPointer>
#include <OgreVector3.h>

namespace Ogre
{
class SceneNode;
class Camera;
}

// Handles camera updates, operating in a thread-safe manner on the QOEngine's thread.
class CameraHandler : public QObject
{
    Q_OBJECT
public:
    CameraHandler();

    qreal getZoom() const;
signals:
    void zoomChanged(qreal zoomLevel);
    void focusNodeChanged(Ogre::SceneNode* node);
    void setupChanged(Ogre::Camera* camera, Ogre::SceneNode* node);
public slots:
    void onCreateCameraWithCurrentSceneManager();
    void onRelativeYawChanged(qreal yaw);
    void onRelativePitchChanged(qreal pitch);
    void onZoomChanged(qreal zoom);
    void onFocusSceneNode(Ogre::SceneNode* node);
private:
    void fitToContain(Ogre::SceneNode* node);
    float getDistanceToAutoTrackingTarget() const;

    float mInitialDistance;
    Ogre::SceneNode* mNode;
    Ogre::Camera* mCamera;
};

class DLL_EXPORT QOCamera : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal zoom READ getZoom WRITE zoom)
public:
    explicit QOCamera(QObject *parent = 0);

    void createCameraWithCurrentSceneManager();

    Ogre::SceneNode* sceneNode() const;
    Ogre::SceneNode* focusedNode() const;
    Ogre::Camera*    camera() const;

    // Thread-safe
    Q_INVOKABLE void yaw(qreal y);   // Radians
    Q_INVOKABLE void pitch(qreal p); // Radians
    Q_INVOKABLE void zoom(qreal z);  // Times of original distance (fit to contain = 1)
    qreal getZoom() const;

    // This will cause the Ogre::Camera to depend on this scene node.
    // NOTE: You must change the camera focus once a scene node is removed.
    Q_INVOKABLE void focus(Ogre::SceneNode* node);
signals:
    void cameraChanged(Ogre::Camera* camera);
    void requestCreateCameraWithCurrentSceneManager();
    void requestRelativeYawChanged(qreal y);
    void requestRelativePitchChanged(qreal p);
    void requestZoomChanged(qreal z);
    void requestFocusSceneNode(Ogre::SceneNode* node);
public slots:
    void onZoomChanged(qreal zoom);
    void onFocusNodeChanged(Ogre::SceneNode* focusNode);
    void onSetupChanged(Ogre::Camera* camera, Ogre::SceneNode* sceneNode);
private:
    qreal mZoom;
    Ogre::SceneNode* mNode;
    Ogre::SceneNode* mFocusNode;
    Ogre::Camera*    mCamera;
    QScopedPointer<CameraHandler> mHandler;
};

#endif // QOCamera_H
