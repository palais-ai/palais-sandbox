/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "QOItem.h"
#include "QONode.h"
#include "QOEngine.h"
#include "QOCamera.h"
#include <OgreCamera.h>
#include <QQuickItemGrabResult>

QOItem::QOItem(QQuickItem *parent)
    : QQuickItem(parent)
    , mBackgroundColor(QColor::fromRgbF(0,0,0))
    , mCamera(NULL)
    , mLastNode(NULL)
    , mEngineItem(NULL)
{
    setFlag(ItemHasContents);
    setSmooth(false);

    connect(this, &QQuickItem::windowChanged,
            this, &QOItem::windowChanged);
}

void QOItem::windowChanged(QQuickWindow *window)
{
    connect(window, &QQuickWindow::frameSwapped,
            this, &QOItem::update);
}

QColor QOItem::backgroundColor() const
{
    return mBackgroundColor;
}

void QOItem::setBackgroundColor(QColor color)
{
    mBackgroundColor = color;
}

QOEngine* QOItem::engine() const
{
    return mEngineItem;
}

QObject* QOItem::camera() const
{
    return dynamic_cast<QObject*>(mCamera);
}

QImage QOItem::saveCurrentImage()
{
    return mLastNode->renderToImage();
}

QSGNode* QOItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (width() <= 0 || height() <= 0 || !mCamera || !mCamera->camera() || !mEngineItem)
    {
        delete oldNode;
        return 0;
    }

    QONode *node = static_cast<QONode *>(oldNode);
    if (!node)
    {
        node = mLastNode = new QONode();
        node->setCamera(mCamera->camera());
        node->setQOEngineItem(mEngineItem);
        connect(mCamera, &QOCamera::cameraChanged,
                mLastNode, &QONode::onCameraChanged);
    }

    node->setSize(QSize(width(), height()));
    node->setBackgroundColor(mBackgroundColor);
    node->update();

    // mark texture dirty, otherwise Qt will not trigger a redraw (preprocess())
    node->markDirty(QSGNode::DirtyMaterial);

    return node;
}

void QOItem::setCamera(QObject *camera)
{
    if(mCamera && mLastNode)
    {
        disconnect(mCamera, &QOCamera::cameraChanged,
                   mLastNode, &QONode::onCameraChanged);
    }

    mCamera = dynamic_cast<QOCamera*>(camera);

    if(mLastNode)
    {
        mLastNode->setCamera(mCamera->camera());
        connect(mCamera, &QOCamera::cameraChanged,
                mLastNode, &QONode::onCameraChanged);
    }
}

void QOItem::setEngine(QOEngine *QOEngine)
{
    mEngineItem = QOEngine;
}
