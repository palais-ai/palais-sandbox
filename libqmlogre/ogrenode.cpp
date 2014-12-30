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

#include "ogrenode.h"

#include <sstream>
#include <cfloat>

#include <Ogre.h>
#include <OgreString.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QTime>

OgreNode::OgreNode(float fboCreationDelay)
    : QSGGeometryNode()
    , m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4)
    , m_texture(0)
    , m_ogreEngineItem(0)
    , m_backgroundColor(QColor::fromRgbF(0,0,0))
    , m_camera(0)
    , m_renderTarget(0)
    , m_ogreFboId(0)
    , m_fboCreationDelay(fboCreationDelay)
    , m_fboDelayAccumulator(-1.f)
    , m_dirtyFBO(false)
{
    setMaterial(&m_material);
    setOpaqueMaterial(&m_materialO);
    setGeometry(&m_geometry);
    setFlag(UsePreprocess);
}

OgreNode::~OgreNode()
{
    if(m_renderTarget)
    {
        m_renderTarget->removeAllViewports();
    }

    if(Ogre::Root::getSingletonPtr())
    {
        Ogre::Root::getSingletonPtr()->detachRenderTarget(m_renderTarget);
    }

    if(m_renderTarget)
    {
        Ogre::TextureManager::getSingleton().remove("RttTex");
    }

    if(m_texture)
    {
        delete m_texture;
    }
}

void OgreNode::setOgreEngineItem(OgreEngine *ogreRootItem)
{
    m_ogreEngineItem = ogreRootItem;
}

void OgreNode::doneOgreContext()
{
    if (m_ogreFboId != 0)
    {
        Ogre::GLFrameBufferObject* ogreFbo = NULL;
        m_renderTarget->getCustomAttribute("FBO", &ogreFbo);
        Ogre::GLFBOManager* manager = ogreFbo->getManager();
        manager->unbind(m_renderTarget);
    }

    m_ogreEngineItem->doneOgreContext();
}

void OgreNode::activateOgreContext()
{
    m_ogreEngineItem->activateOgreContext();
    m_ogreEngineItem->ogreContext()->functions()->glBindFramebuffer(GL_FRAMEBUFFER_EXT,
                                                                    m_ogreFboId);
}

GLuint OgreNode::getOgreFboId()
{
    if (!m_renderTarget)
    {
        qWarning("No render target present, but FBO was requested.");
        return 0;
    }

    Ogre::GLFrameBufferObject* ogreFbo = NULL;
    m_renderTarget->getCustomAttribute("FBO", &ogreFbo);
    Ogre::GLFBOManager* manager = ogreFbo->getManager();
    manager->bind(m_renderTarget);

    GLint id;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &id);

    return id;
}

void OgreNode::resetViewport()
{
    if(!m_renderTarget || !m_camera)
    {
        qWarning("No render target or camera.");
        return;
    }

    Ogre::Real aspectRatio = Ogre::Real(m_size.width()) / Ogre::Real(m_size.height());
    m_camera->setAspectRatio(aspectRatio);
    m_renderTarget->removeAllViewports();

    m_renderTarget->addViewport(m_camera);
    m_renderTarget->getViewport(0)->setClearEveryFrame(true);
    m_renderTarget->getViewport(0)->setBackgroundColour(Ogre::ColourValue(m_backgroundColor.redF(),
                                                                          m_backgroundColor.greenF(),
                                                                          m_backgroundColor.blueF()));
    m_renderTarget->getViewport(0)->setOverlaysEnabled(false);
}

void OgreNode::preprocess()
{
    m_ogreEngineItem->lockEngine();

    if (!m_renderTarget)
    {
        qWarning("No render target.");
        return;
    }

    activateOgreContext();

    resetViewport();

    m_ogreEngineItem->getRoot()->_fireFrameStarted();
    m_ogreEngineItem->getRoot()->_fireFrameRenderingQueued();
    m_renderTarget->update();
    m_ogreEngineItem->getRoot()->_fireFrameEnded();

    doneOgreContext();

    m_ogreEngineItem->unlockEngine();
}

void OgreNode::update()
{
    static QTime last = QTime::currentTime();

    QTime now = QTime::currentTime();
    if(m_fboDelayAccumulator > 0.f ||
       fabs(m_fboDelayAccumulator) < FLT_EPSILON)
    {
        m_fboDelayAccumulator -= (last.msecsTo(now) / 1000.f);

        // Delay ran out in this frame - update the frame buffer to its latest size.
        if(m_fboDelayAccumulator < 0.f)
        {
            m_dirtyFBO = true;
        }
    }
    last = now;

    if (m_dirtyFBO)
    {
        m_ogreEngineItem->lockEngine();

        activateOgreContext();
        updateFBO();
        m_ogreFboId = getOgreFboId();
        m_dirtyFBO = false;
        doneOgreContext();

        m_ogreEngineItem->unlockEngine();
    }
}

int OgreNode::getNumberOfFSAASamples()
{
    int samples = 0;
    try
    {
        Ogre::RenderSystem* renderSystem = m_ogreEngineItem->getRoot()->getRenderSystem();
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

void OgreNode::updateFBO()
{
    static const Ogre::String textureName = "RttTex";

    if (m_renderTarget)
    {
        Ogre::TextureManager::getSingleton().remove(textureName);
    }

    int samples = getNumberOfFSAASamples();

    // Don't recreate the texture on every frame during animations / Continuous size changes
    if(m_fboDelayAccumulator < 0.f)
    {
        m_rttTexture = Ogre::TextureManager::getSingleton().createManual(textureName,
                                                                         Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                                         Ogre::TEX_TYPE_2D,
                                                                         m_size.width(),
                                                                         m_size.height(),
                                                                         0, // num mipmaps
                                                                         Ogre::PF_R8G8B8A8,
                                                                         Ogre::TU_RENDERTARGET,
                                                                         0, // ManualLoader
                                                                         false, // hwGammaCorrection
                                                                         samples);

        m_fboDelayAccumulator = m_fboCreationDelay;
    }

    m_renderTarget = m_rttTexture->getBuffer()->getRenderTarget();
    m_renderTarget->setActive(true);

    resetViewport();

    QSGGeometry::updateTexturedRectGeometry(&m_geometry,
                                            QRectF(0, 0, m_size.width(), m_size.height()),
                                            QRectF(0, 0, 1, 1));

    Ogre::GLTexture* nativeTexture = static_cast<Ogre::GLTexture*>(m_rttTexture.get());

    if(m_texture)
    {
        delete m_texture;
    }

    m_texture = m_ogreEngineItem->createTextureFromId(nativeTexture->getGLID(), m_size);

    m_material.setTexture(m_texture);
    m_materialO.setTexture(m_texture);
}

void OgreNode::setCamera(Ogre::Camera *camera)
{
    m_camera = camera;
}

void OgreNode::setBackgroundColor(QColor color)
{
    m_backgroundColor = color;
}

QSize OgreNode::size() const
{
    return m_size;
}

void OgreNode::setSize(const QSize &size)
{
    if (size == m_size)
    {
        return;
    }

    m_size = size;
    m_dirtyFBO = true;
    markDirty(DirtyGeometry);
}
