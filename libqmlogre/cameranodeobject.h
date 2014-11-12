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

#include <QObject>

#include "ogrecamerawrapper.h"

namespace Ogre {
class SceneNode;
class Camera;
}

class CameraNodeObject : public QObject, public OgreCameraWrapper
{
    Q_OBJECT
    Q_PROPERTY(qreal yaw READ yaw WRITE setYaw)
    Q_PROPERTY(qreal pitch READ pitch WRITE setPitch)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom)
    Q_PROPERTY(bool wireframeMode READ getWireframeMode WRITE setWireframeMode)
public:
    explicit CameraNodeObject(QObject *parent = 0);

    Ogre::SceneNode *sceneNode() const
    { return m_node; }
    Ogre::Camera* camera() const { return m_camera; }

    qreal yaw() const
    { return m_yaw; }
    qreal pitch() const
    { return m_pitch; }
    qreal zoom() const
    { return m_zoom; }
    void setYaw(qreal y)
    { m_yaw = y; updateRotation(); }
    void setPitch(qreal p)
    { m_pitch = p; updateRotation(); }
    void setZoom(qreal z);
    void setWireframeMode(bool enabled);
    bool getWireframeMode() const;

private:
    void updateRotation();

    Ogre::SceneNode *m_node;
    Ogre::Camera *m_camera;

    qreal m_yaw;
    qreal m_pitch;
    qreal m_zoom;
};

#endif // CAMERANODEOBJECT_H
