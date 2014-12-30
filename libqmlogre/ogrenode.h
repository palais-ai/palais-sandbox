/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#ifndef OGRENODE_H
#define OGRENODE_H

#include "ogreengine.h"

#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGTextureMaterial>
#include <QtQuick/QSGOpaqueTextureMaterial>

#include <OgreTexture.h>

namespace Ogre
{
class Root;
class Camera;
class SceneManager;
class RenderTexture;
class Viewport;
class RenderTarget;
class RenderWindow;
}

class CameraNodeObject;

class OgreNode : public QSGGeometryNode
{
public:
    // The fboCreationDelay is used to throttle the framebuffer recreation rate
    // when this QQuickItem's size is changed continuously.
    OgreNode(float fboCreationDelay = 1.f);
    ~OgreNode();

    void setSize(const QSize &size);
    QSize size() const;

    void update();
    void updateFBO();

    GLuint getOgreFboId();

    void setOgreEngineItem(OgreEngine* ogreRootItem);
    void doneOgreContext();
    void activateOgreContext();

    void preprocess();

    void setCamera(Ogre::Camera* camera);
    void setBackgroundColor(QColor color);

private:
    void resetViewport();
    int getNumberOfFSAASamples();

    QSGTextureMaterial m_material;
    QSGOpaqueTextureMaterial m_materialO;
    QSGGeometry m_geometry;
    QSGTexture* m_texture;
    OgreEngine* m_ogreEngineItem;

    QColor m_backgroundColor;
    QSize m_size;

    Ogre::Camera* m_camera;
    Ogre::RenderTexture* m_renderTarget;
    Ogre::TexturePtr m_rttTexture;

    GLuint m_ogreFboId;
    const float m_fboCreationDelay; // In seconds
    float m_fboDelayAccumulator;
    bool m_dirtyFBO;
};

#endif // OGRENODE_H
