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

OgreItem::OgreItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_backgroundColor(QColor::fromRgbF(0,0,0))
    , m_camera(0)
    , mLastNode(0)
    , m_ogreEngineItem(0)
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
    return m_backgroundColor;
}

void OgreItem::setBackgroundColor(QColor color)
{
    m_backgroundColor = color;
}

OgreEngine* OgreItem::ogreEngine() const
{
    return m_ogreEngineItem;
}

QObject* OgreItem::camera() const
{
    return dynamic_cast<QObject*>(m_camera);
}

QImage OgreItem::saveCurrentImage()
{
    return mLastNode->renderToImage();
}

QSGNode* OgreItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (width() <= 0 || height() <= 0 || !m_camera || !m_camera->camera() || !m_ogreEngineItem)
    {
        delete oldNode;
        return 0;
    }

    OgreNode *node = static_cast<OgreNode *>(oldNode);
    if (!node)
    {
        node = mLastNode = new OgreNode();
        node->setCamera(m_camera->camera());
        node->setOgreEngineItem(m_ogreEngineItem);
        connect(m_camera, &CameraNodeObject::cameraChanged,
                mLastNode, &OgreNode::onCameraChanged);
    }

    node->setSize(QSize(width(), height()));
    node->setBackgroundColor(m_backgroundColor);
    node->update();

    // mark texture dirty, otherwise Qt will not trigger a redraw (preprocess())
    node->markDirty(QSGNode::DirtyMaterial);

    return node;
}

void OgreItem::setCamera(QObject *camera)
{
    if(m_camera && mLastNode)
    {
        disconnect(m_camera, &CameraNodeObject::cameraChanged,
                   mLastNode, &OgreNode::onCameraChanged);
    }

    m_camera = dynamic_cast<CameraNodeObject*>(camera);

    if(mLastNode)
    {
        mLastNode->setCamera(m_camera->camera());
        connect(m_camera, &CameraNodeObject::cameraChanged,
                mLastNode, &OgreNode::onCameraChanged);
    }
}

void OgreItem::setOgreEngine(OgreEngine *ogreEngine)
{
    m_ogreEngineItem = ogreEngine;
}
