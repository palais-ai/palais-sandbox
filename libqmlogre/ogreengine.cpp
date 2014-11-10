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

#include <Ogre.h>
#include <QOpenGLFunctions>
#include <QDir>

static QString appPath()
{
    QString path = QCoreApplication::applicationDirPath();
    QDir dir(path);
#ifdef Q_OS_MAC
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();
#elif defined(Q_OS_WIN)
    dir.cdUp();
#endif
    return dir.absolutePath();
}

OgreEngine::OgreEngine(QQuickWindow *window)
    : QObject(),
      m_resources_cfg(Ogre::StringUtil::BLANK),
      mLoadingProgress(0),
      mInitProportion(1.0),
      mCurrentProgress(0),
      mNumGroupsInit(0),
      mNumGroupsLoad(0)
{
    qmlRegisterType<OgreItem>("Ogre", 1, 0, "OgreItem");
    qmlRegisterType<OgreEngine>("OgreEngine", 1, 0, "OgreEngine");
    qmlRegisterType<CameraNodeObject>("Example", 1, 0, "Camera");

    setQuickWindow(window);
}

OgreEngine::~OgreEngine()
{
    delete m_ogreContext;
}

Ogre::Root* OgreEngine::startEngine()
{
    m_resources_cfg = "resources.cfg";

    activateOgreContext();

    Ogre::Root *ogreRoot = new Ogre::Root;
    Ogre::RenderSystem *renderSystem = ogreRoot->getRenderSystemByName("OpenGL Rendering Subsystem");

#if defined(Q_OS_WIN)
    renderSystem->setConfigOption("FSAA","0");
#endif

    ogreRoot->setRenderSystem(renderSystem);
    ogreRoot->initialise(false);

    Ogre::NameValuePairList params;

    params["externalGLControl"] = "true";
    params["currentGLContext"] = "true";
    params["hidden"] = "true";

#ifdef Q_OS_MAC
    params["macAPI"] = "cocoa";
#endif

    //Finally create our window.
    m_ogreWindow = ogreRoot->createRenderWindow("OgreWindow", 1, 1, false, &params);
    m_ogreWindow->setVisible(false);
    m_ogreWindow->update(false);

    doneOgreContext();

    return ogreRoot;
}

void OgreEngine::stopEngine(Ogre::Root *ogreRoot)
{
    if (ogreRoot) {
//        m_root->detachRenderTarget(m_renderTexture);
        // TODO tell node(s) to detach

    }

    delete ogreRoot;
}

void OgreEngine::setQuickWindow(QQuickWindow *window)
{
    Q_ASSERT(window);

    m_quickWindow = window;
    m_qtContext = QOpenGLContext::currentContext();

    // create a new shared OpenGL context to be used exclusively by Ogre
    /*m_ogreContext = new QOpenGLContext();
    m_ogreContext->setFormat(m_quickWindow->requestedFormat());
    m_ogreContext->setShareContext(m_qtContext);
    m_ogreContext->create();*/

    m_ogreContext = m_qtContext;
}

void OgreEngine::activateOgreContext()
{
    glPopAttrib();
    glPopClientAttrib();

    m_qtContext->functions()->glUseProgram(0);
    m_qtContext->doneCurrent();

    m_ogreContext->makeCurrent(m_quickWindow);
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
    Ogre::ResourceGroupManager::getSingleton().addResourceGroupListener(this);

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

            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

    mNumGroupsInit = 1;
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void OgreEngine::resourceGroupScriptingStarted(const Ogre::String& groupName, size_t scriptCount)
{
    assert(mNumGroupsInit > 0 && "You stated you were not going to init "
        "any groups, but you did! Divide by zero would follow...");

    if(scriptCount == 0)
    {
        mCurrentProgress = 0;
        return;
    }

    mCurrentProgress = 100 * mInitProportion / (Ogre::Real)scriptCount;
    mCurrentProgress /= mNumGroupsInit;
}

void OgreEngine::scriptParseStarted(const Ogre::String& scriptName, bool &skipThisScript)
{
    ;
}

void OgreEngine::scriptParseEnded(const Ogre::String& scriptName, bool skipped)
{
    addLoadingProgress(mCurrentProgress);
}

void OgreEngine::resourceGroupScriptingEnded(const Ogre::String& groupName)
{
}

void OgreEngine::resourceGroupLoadStarted(const Ogre::String& groupName, size_t resourceCount)
{
    assert(mNumGroupsLoad > 0 && "You stated you were not going to load "
        "any groups, but you did! Divide by zero would follow...");

    if(resourceCount == 0)
    {
        mCurrentProgress = 0;
        return;
    }

    mCurrentProgress = 100 * (1-mInitProportion) / (Ogre::Real)resourceCount;
    mCurrentProgress /= mNumGroupsLoad;
}

void OgreEngine::resourceLoadStarted(const Ogre::ResourcePtr& resource)
{
    ;
}

void OgreEngine::resourceLoadEnded(void)
{
}

void OgreEngine::worldGeometryStageStarted(const Ogre::String& description)
{
    ;
}

void OgreEngine::worldGeometryStageEnded(void)
{
    addLoadingProgress(mCurrentProgress);
}

void OgreEngine::resourceGroupLoadEnded(const Ogre::String& groupName)
{
}

