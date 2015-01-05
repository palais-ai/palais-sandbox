/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#ifndef CAMERANODEOBJECT_H
#define CAMERANODEOBJECT_H

#include "qmlogre_global.h"

#include <QObject>
#include <OgreVector3.h>

namespace Ogre
{
class SceneNode;
class Camera;
}

class DLL_EXPORT CameraNodeObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal yaw READ yaw WRITE setYaw)
    Q_PROPERTY(qreal pitch READ pitch WRITE setPitch)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom)
    Q_PROPERTY(bool wireframeMode READ getWireframeMode WRITE setWireframeMode)
public:
    explicit CameraNodeObject(QObject *parent = 0);

    void createCameraWithCurrentSceneManager();

    Ogre::SceneNode* sceneNode() const;
    Ogre::Camera* camera() const;

    // Thread-safe
    qreal yaw() const;
    qreal pitch() const;
    qreal zoom() const;
    void setYaw(qreal y);
    void setPitch(qreal p);
    void setZoom(qreal z);

    void setWireframeMode(bool enabled);
    bool getWireframeMode() const;

    // Not thread-safe
    void fitToContain(Ogre::SceneNode* node);

    // Thread-safe
    void focus(Ogre::SceneNode* node);

signals:
    void cameraChanged(Ogre::Camera* camera);
private:
    void updateRotation();

    Ogre::Vector3 mInitialPosition;

    Ogre::SceneNode* m_node;
    Ogre::Camera* m_camera;

    qreal m_yaw;
    qreal m_pitch;
    qreal m_zoom;
};

#endif // CAMERANODEOBJECT_H
