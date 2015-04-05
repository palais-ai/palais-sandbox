/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include <RenderSystems/GL/OgreGLTexture.h>
#include <RenderSystems/GL/OgreGLFrameBufferObject.h>
#include <RenderSystems/GL/OgreGLFBORenderTexture.h>
#include "QONode.h"
#include <sstream>
#include <cfloat>
#include <Ogre.h>
#include <OgreString.h>
#include <OgreRenderTarget.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTime>

QONode::QONode(float fboCreationDelay)
    : QSGGeometryNode()
    , mGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4)
    , mTexture(NULL)
    , mQOEngineItem(NULL)
    , mBackgroundColor(QColor::fromRgbF(0,0,0))
    , mCamera(NULL)
    , mRenderTarget(NULL)
    , mOgreFBOId(0)
    , mFBOCreationDelay(fboCreationDelay)
    , mFBODelayAccumulator(-1.f)
    , mDirtyFBO(false)
{
    setMaterial(&mMaterial);
    setOpaqueMaterial(&mOpaqueMaterial);
    setGeometry(&mGeometry);
    setFlag(UsePreprocess);
}

QONode::~QONode()
{
    if(mRenderTarget)
    {
        mRenderTarget->removeAllViewports();
    }

    if(Ogre::Root::getSingletonPtr())
    {
        Ogre::Root::getSingletonPtr()->detachRenderTarget(mRenderTarget);
    }

    if(mRenderTarget)
    {
        Ogre::TextureManager::getSingleton().remove("RttTex");
    }
}

void QONode::setQOEngineItem(QOEngine *ogreRootItem)
{
    mQOEngineItem = ogreRootItem;
}

void QONode::doneOgreContext()
{
    if (mOgreFBOId != 0)
    {
        Ogre::GLFrameBufferObject* ogreFbo = NULL;
        mRenderTarget->getCustomAttribute("FBO", &ogreFbo);
        Ogre::GLFBOManager* manager = ogreFbo->getManager();
        manager->unbind(mRenderTarget);
    }

    mQOEngineItem->doneOgreContext();
}

void QONode::activateOgreContext()
{
    mQOEngineItem->activateOgreContext();
    mQOEngineItem->ogreContext()->functions()->glBindFramebuffer(GL_FRAMEBUFFER_EXT,
                                                                    mOgreFBOId);
}

GLuint QONode::getOgreFboId()
{
    if (!mRenderTarget)
    {
        qWarning("No render target present, but FBO was requested.");
        return 0;
    }

    Ogre::GLFrameBufferObject* ogreFbo = NULL;
    mRenderTarget->getCustomAttribute("FBO", &ogreFbo);
    Ogre::GLFBOManager* manager = ogreFbo->getManager();
    manager->bind(mRenderTarget);

    GLint id;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);

    return id;
}

void QONode::resetViewport()
{
    if(!mRenderTarget || !mCamera)
    {
        qWarning("No render target or camera.");
        return;
    }

    Ogre::Real aspectRatio = Ogre::Real(mSize.width()) / Ogre::Real(mSize.height());
    mCamera->setAspectRatio(aspectRatio);
    mRenderTarget->removeAllViewports();

    mRenderTarget->addViewport(mCamera);
    mRenderTarget->getViewport(0)->setClearEveryFrame(true);

    const Ogre::ColourValue bgColor(mBackgroundColor.redF(),
                                    mBackgroundColor.greenF(),
                                    mBackgroundColor.blueF());
    mRenderTarget->getViewport(0)->setBackgroundColour(bgColor);
    mRenderTarget->getViewport(0)->setOverlaysEnabled(false);
}

QImage QONode::renderToImage()
{
    const uint32_t w = mRenderTarget->getWidth();
    const uint32_t h = mRenderTarget->getHeight();

    QImage retVal(w, h, QImage::Format_RGBA8888);

    if(retVal.isNull())
    {
        // Callers handle the allocation error.
        return retVal;
    }

    // Set dest format
    const Ogre::PixelBox encodeDest = Ogre::PixelBox(w,
                                                     h,
                                                     1,
                                                     Ogre::PF_A8B8G8R8,
                                                     retVal.bits());

    // Do the copy, NOTE: Differs from the slides (other ogre version?)
    mRenderTarget->copyContentsToMemory(encodeDest, Ogre::RenderTarget::FB_AUTO);

    return retVal;
}

void QONode::preprocess()
{
    if(!mQOEngineItem || !mCamera)
    {
        qWarning("No engine or camera supplied to QONode.");
        return;
    }

    if (!mRenderTarget)
    {
        qWarning("No render target.");
        return;
    }

    activateOgreContext();
    resetViewport();

    mQOEngineItem->getRoot()->_fireFrameStarted();
    mQOEngineItem->getRoot()->_fireFrameRenderingQueued();
    mRenderTarget->update();
    mQOEngineItem->getRoot()->_fireFrameEnded();

    doneOgreContext();
}

void QONode::update()
{
    static QTime last = QTime::currentTime();

    QTime now = QTime::currentTime();
    if(mFBODelayAccumulator > 0.f ||
       fabs(mFBODelayAccumulator) < FLT_EPSILON)
    {
        mFBODelayAccumulator -= (last.msecsTo(now) / 1000.f);

        // Delay ran out in this frame - update the frame buffer to its latest size.
        if(mFBODelayAccumulator < 0.f)
        {
            mDirtyFBO = true;
        }
    }
    last = now;

    if (mDirtyFBO)
    {
        activateOgreContext();
        updateFBO();
        mOgreFBOId = getOgreFboId();
        mDirtyFBO = false;
        doneOgreContext();
    }
}

int QONode::getNumberOfFSAASamples()
{
    int samples = 0;
    try
    {
        Ogre::RenderSystem* renderSystem = mQOEngineItem->getRoot()->getRenderSystem();
        Ogre::String samplesStr = renderSystem->getConfigOptions()["FSAA"].currentValue;
        std::stringstream ss(samplesStr);
        ss >> samples;

        if(ss.fail())
        {
            samples = 0;
        }
    }
    catch(...)
    {
        samples = 0;
    }

    return samples;
}

void QONode::updateFBO()
{
    static const Ogre::String textureName = "RttTex";

    if (mRenderTarget)
    {
        Ogre::TextureManager::getSingleton().remove(textureName);
    }

    int samples = getNumberOfFSAASamples();

    // Don't recreate the texture on every frame during animations / Continuous size changes.
    if(mFBODelayAccumulator < 0.f)
    {
        Ogre::TextureManager& texMgr = Ogre::TextureManager::getSingleton();
        mRTTTexture = texMgr.createManual(textureName,
                                           Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                           Ogre::TEX_TYPE_2D,
                                           mSize.width(),
                                           mSize.height(),
                                           0, // num mipmaps
                                           Ogre::PF_FLOAT32_RGBA,
                                           Ogre::TU_RENDERTARGET,
                                           0, // ManualLoader
                                           false, // hwGammaCorrection
                                           samples);

        mFBODelayAccumulator = mFBOCreationDelay;
    }

    mRenderTarget = mRTTTexture->getBuffer()->getRenderTarget();
    mRenderTarget->setActive(true);

    resetViewport();

    QSGGeometry::updateTexturedRectGeometry(&mGeometry,
                                            QRectF(0, 0, mSize.width(), mSize.height()),
                                            QRectF(0, 0, 1, 1));

    Ogre::GLTexture* nativeTexture = static_cast<Ogre::GLTexture*>(mRTTTexture.get());

    mTexture.reset(mQOEngineItem->createTextureFromId(nativeTexture->getGLID(), mSize));

    mMaterial.setTexture(mTexture.data());
    mOpaqueMaterial.setTexture(mTexture.data());
}

void QONode::onCameraChanged(Ogre::Camera* newCamera)
{
    mCamera = newCamera;
}

void QONode::setCamera(Ogre::Camera *camera)
{
    mCamera = camera;
}

void QONode::setBackgroundColor(QColor color)
{
    mBackgroundColor = color;
}

QSize QONode::size() const
{
    return mSize;
}

void QONode::setSize(const QSize &size)
{
    if (size == mSize)
    {
        return;
    }

    mSize = size;
    mDirtyFBO = true;
    markDirty(DirtyGeometry);
}
