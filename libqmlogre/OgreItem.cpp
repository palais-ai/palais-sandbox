/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "ogreitem.h"
#include "ogrenode.h"
#include "ogreengine.h"
#include "cameranodeobject.h"
#include <OgreCamera.h>
#include <QQuickItemGrabResult>

OgreItem::OgreItem(QQuickItem *parent)
    : QQuickItem(parent)
    , mBackgroundColor(QColor::fromRgbF(0,0,0))
    , mCamera(NULL)
    , mLastNode(NULL)
    , mOgreEngineItem(NULL)
{
    setFlag(ItemHasContents);
    setSmooth(false);

    connect(this, &QQuickItem::windowChanged,
            this, &OgreItem::windowChanged);
}

void OgreItem::windowChanged(QQuickWindow *window)
{
    connect(window, &QQuickWindow::frameSwapped,
            this, &OgreItem::update);
}

QColor OgreItem::backgroundColor() const
{
    return mBackgroundColor;
}

void OgreItem::setBackgroundColor(QColor color)
{
    mBackgroundColor = color;
}

OgreEngine* OgreItem::ogreEngine() const
{
    return mOgreEngineItem;
}

QObject* OgreItem::camera() const
{
    return dynamic_cast<QObject*>(mCamera);
}

QImage OgreItem::saveCurrentImage()
{
    return mLastNode->renderToImage();
}

QSGNode* OgreItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (width() <= 0 || height() <= 0 || !mCamera || !mCamera->camera() || !mOgreEngineItem)
    {
        delete oldNode;
        return 0;
    }

    OgreNode *node = static_cast<OgreNode *>(oldNode);
    if (!node)
    {
        node = mLastNode = new OgreNode();
        node->setCamera(mCamera->camera());
        node->setOgreEngineItem(mOgreEngineItem);
        connect(mCamera, &CameraNodeObject::cameraChanged,
                mLastNode, &OgreNode::onCameraChanged);
    }

    node->setSize(QSize(width(), height()));
    node->setBackgroundColor(mBackgroundColor);
    node->update();

    // mark texture dirty, otherwise Qt will not trigger a redraw (preprocess())
    node->markDirty(QSGNode::DirtyMaterial);

    return node;
}

void OgreItem::setCamera(QObject *camera)
{
    if(mCamera && mLastNode)
    {
        disconnect(mCamera, &CameraNodeObject::cameraChanged,
                   mLastNode, &OgreNode::onCameraChanged);
    }

    mCamera = dynamic_cast<CameraNodeObject*>(camera);

    if(mLastNode)
    {
        mLastNode->setCamera(mCamera->camera());
        connect(mCamera, &CameraNodeObject::cameraChanged,
                mLastNode, &OgreNode::onCameraChanged);
    }
}

void OgreItem::setOgreEngine(OgreEngine *ogreEngine)
{
    mOgreEngineItem = ogreEngine;
}
