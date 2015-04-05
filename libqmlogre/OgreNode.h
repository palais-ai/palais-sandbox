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

#include "qmlogre_global.h"
#include "OgreEngine.h"
#include <QObject>
#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGTextureMaterial>
#include <QtQuick/QSGOpaqueTextureMaterial>
#include <QScopedPointer>

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

class DLL_EXPORT OgreNode : public QObject,
                            public QSGGeometryNode
{
    Q_OBJECT
public:
    // The fboCreationDelay is used to throttle the framebuffer recreation rate
    // when this QQuickItem's size is changed continuously.
    OgreNode(float fboCreationDelay = 1.f); // In seconds
    ~OgreNode();

    void setSize(const QSize &size);
    QSize size() const;

    void update();
    void updateFBO();

    // QImage's "isNull" returns true if the allocation failed.
    QImage renderToImage();
    GLuint getOgreFboId();

    void setOgreEngineItem(OgreEngine* ogreRootItem);
    void doneOgreContext();
    void activateOgreContext();

    void preprocess();

    void setCamera(Ogre::Camera* camera);
    void setBackgroundColor(QColor color);

public slots:
    void onCameraChanged(Ogre::Camera* newCamera);
private:
    void resetViewport();
    int getNumberOfFSAASamples();

    QSGTextureMaterial mMaterial;
    QSGOpaqueTextureMaterial mMaterial0;
    QSGGeometry mGeometry;
    QScopedPointer<QSGTexture> mTexture;
    OgreEngine* mOgreEngineItem;

    QColor mBackgroundColor;
    QSize mSize;

    Ogre::Camera* mCamera;
    Ogre::RenderTexture* mRenderTarget;
    Ogre::TexturePtr mRTTTexture;

    GLuint mOgreFBOId;
    const float mFBOCreationDelay; // In seconds
    float mFBODelayAccumulator;
    bool mDirtyFBO;
};

#endif // OGRENODE_H
