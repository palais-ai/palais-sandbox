/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "ogreengine.h"
#include "ogreitem.h"
#include "cameranodeobject.h"

#include <exception>

#include <OgreAnimation.h>

#include <QDebug>
#include <Ogre.h>
#include <QOpenGLFunctions>
#include <QDir>

QMutex g_engineMutex;

OgreEngine::OgreEngine(QQuickWindow *window)
    : QObject(),
      m_resources_cfg(Ogre::StringUtil::BLANK),
      mRoot(0)
{
    qmlRegisterType<OgreItem>("Ogre", 1, 0, "OgreItem");
    qmlRegisterType<OgreEngine>("OgreEngine", 1, 0, "OgreEngine");
    qmlRegisterType<CameraNodeObject>("Example", 1, 0, "Camera");

    setQuickWindow(window);
}

OgreEngine::~OgreEngine()
{
    if(m_ogreContext && m_ogreContext != m_qtContext)
    {
        delete m_ogreContext;
    }
}

Ogre::Root* OgreEngine::getRoot()
{
    return mRoot;
}


Ogre::RenderWindow* OgreEngine::getRenderWindow()
{
    return m_ogreWindow;
}

bool OgreEngine::isStarted() const
{
    return mRoot;
}

void OgreEngine::startEngine()
{
    activateOgreContext();

    QString basePath = QCoreApplication::applicationDirPath();
    qDebug() << "The base path is " << basePath;

    m_resources_cfg = (basePath + "/resources.cfg").toStdString();
    qDebug() << "Resources path is " << QString::fromStdString(m_resources_cfg);

    Ogre::Root *ogreRoot = new Ogre::Root("", m_resources_cfg);

#if defined(Q_OS_MAC)
    const QString additionalPluginPath = "/../Plugins";
#else
    const QString additionalPluginPath = "";
#endif

#if (defined(DEBUG) && defined(Q_OS_WIN)) // We dont use debug dynamic libraries on mac.
    const std::string renderSystemPath = (basePath + additionalPluginPath + "/RenderSystem_GL_d").toStdString();
    const std::string octtreePath = (basePath + additionalPluginPath + "/Plugin_OctreeSceneManager_d").toStdString();
#else
    const std::string renderSystemPath = (basePath + additionalPluginPath + "/RenderSystem_GL").toStdString();
    const std::string octtreePath = (basePath + additionalPluginPath + "/Plugin_OctreeSceneManager").toStdString();
#endif

    qDebug() << "Render System path is " << QString::fromStdString(renderSystemPath);
    qDebug() << "Octtree Plugin path is " << QString::fromStdString(octtreePath);

    ogreRoot->loadPlugin(renderSystemPath);
    ogreRoot->loadPlugin(octtreePath);

    Ogre::RenderSystem *renderSystem = ogreRoot->getRenderSystemByName("OpenGL Rendering Subsystem");

#if defined(Q_OS_WIN)
    renderSystem->setConfigOption("FSAA","0");
#endif

    if(!renderSystem)
    {
        throw std::runtime_error("Render system not found. The RenderSystemGL shared library is most likely not being found.");
    }

    ogreRoot->setRenderSystem(renderSystem);
    ogreRoot->initialise(false);

    // Get the parameters of the window QT created
    Ogre::String winHandle;
#ifdef defined(Q_OS_WIN)
    // Windows code
    winHandle += Ogre::StringConverter::toString((unsigned long)(m_quickWindow->parent()->winId()));
#elif defined(Q_OS_MAC)
    // Mac code, tested on Mac OSX 10.6 using Qt 4.7.4 and Ogre 1.7.3
    winHandle  = Ogre::StringConverter::toString((unsigned long)m_quickWindow->winId());
#else
    // Unix code
    QX11Info info = x11Info();
    winHandle  = Ogre::StringConverter::toString((unsigned long)(info.display()));
    winHandle += ":";
    winHandle += Ogre::StringConverter::toString((unsigned int)(info.screen()));
    winHandle += ":";
    winHandle += Ogre::StringConverter::toString((unsigned long)(this->parent()->winId()));
#endif

    Ogre::NameValuePairList params;

    params["externalGLControl"] = "true";
    params["currentGLContext"] = "true";
    params["hidden"] = "true";

#if defined(Q_OS_MAC)
    params["macAPI"] = "cocoa";
    params["macAPICocoaUseNSView"] = "true";
    params["externalWindowHandle"] = winHandle;
#else
    params["parentWindowHandle"] = winHandle;
#endif

    //Finally create our window.
    m_ogreWindow = ogreRoot->createRenderWindow("OgreWindow", 1, 1, false, &params);
    m_ogreWindow->setActive(true);
    m_ogreWindow->setVisible(false);
    m_ogreWindow->update(false);

    Ogre::Animation::setDefaultInterpolationMode(Ogre::Animation::IM_LINEAR);
    Ogre::Animation::setDefaultRotationInterpolationMode(Ogre::Animation::RIM_LINEAR);

    doneOgreContext();

    mRoot = ogreRoot;
}

void OgreEngine::stopEngine()
{
    if (mRoot) {
//        m_root->detachRenderTarget(m_renderTexture);
        // TODO tell node(s) to detach
        delete mRoot;
        mRoot = NULL;
    }
}

void OgreEngine::setQuickWindow(QQuickWindow *window)
{
    Q_ASSERT(window);

    m_quickWindow = window;
    //m_quickWindow->setClearBeforeRendering(true);

    m_qtContext = QOpenGLContext::currentContext();
    m_qtContext->doneCurrent();

    m_ogreContext = m_qtContext;

    // create a new shared OpenGL context to be used exclusively by Ogre
    /**m_ogreContext = new QOpenGLContext();
    m_ogreContext->setFormat(m_quickWindow->requestedFormat());
    m_ogreContext->setShareContext(m_qtContext);
    m_ogreContext->create();

    m_ogreContext->doneCurrent();*/

    m_qtContext->makeCurrent(m_quickWindow);
}

void OgreEngine::activateOgreContext()
{
    glPopAttrib();
    glPopClientAttrib();

    m_qtContext->functions()->glUseProgram(0);
    m_qtContext->doneCurrent();

    m_ogreContext->makeCurrent(m_quickWindow);

    m_quickWindow->resetOpenGLState();
}

void OgreEngine::doneOgreContext()
{
    m_ogreContext->functions()->glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    m_ogreContext->functions()->glBindRenderbuffer(GL_RENDERBUFFER, 0);
    m_ogreContext->functions()->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);

    // unbind all possible remaining buffers; just to be on safe side
    m_ogreContext->functions()->glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef GL_VERSION_4_2
    m_ogreContext->functions()->glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
#endif

    m_ogreContext->functions()->glBindBuffer(GL_COPY_READ_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
//    m_ogreContext->functions()->glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
//    m_ogreContext->functions()->glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_TEXTURE_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
    m_ogreContext->functions()->glBindBuffer(GL_UNIFORM_BUFFER, 0);

    m_ogreContext->doneCurrent();

    m_qtContext->makeCurrent(m_quickWindow);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    m_qtContext->functions()->glUseProgram(0);

    m_quickWindow->resetOpenGLState();
}

void OgreEngine::lockEngine()
{
    g_engineMutex.lock();
}

void OgreEngine::unlockEngine()
{
    g_engineMutex.unlock();
}

QOpenGLContext* OgreEngine::ogreContext() const
{
    return m_ogreContext;
}

QSGTexture* OgreEngine::createTextureFromId(uint id, const QSize &size, QQuickWindow::CreateTextureOptions options) const
{
    return m_quickWindow->createTextureFromId(id, size, options);
}

void OgreEngine::setupResources(void)
{
    QString basePath = QCoreApplication::applicationDirPath();

    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(m_resources_cfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            archName = (basePath + "/" + QString::fromStdString(archName)).toStdString();

            qDebug() << "archName: " << QString::fromStdString(archName);
            qDebug() << "typeName: " << QString::fromStdString(typeName);
            qDebug() << "secName: " << QString::fromStdString(secName);

            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
