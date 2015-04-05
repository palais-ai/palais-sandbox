/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "QOEngine.h"
#include "QOItem.h"
#include "QOCamera.h"
#include <exception>
#include <Ogre.h>
#include <OgreAnimation.h>
#include <QDebug>
#include <QOpenGLFunctions>
#include <QDir>
#include <QQuickWindow>

QThread* g_engineThread;

QOEngine::QOEngine(QQuickWindow *window)
    : QObject(),
      mResourceConfig(Ogre::StringUtil::BLANK),
      mRoot(NULL),
      mQuickWindow(NULL),
      mOgreContext(NULL)
{
    qmlRegisterType<QOItem>("Ogre", 1, 0, "QOItem");
    qmlRegisterType<QOEngine>("QOEngine", 1, 0, "QOEngine");
    qmlRegisterType<QOCamera>("Ogre", 1, 0, "Camera");

    g_engineThread = thread();
    setQuickWindow(window);
}

QOEngine::~QOEngine()
{
    ;
}

Ogre::Root* QOEngine::getRoot()
{
    return mRoot;
}


Ogre::RenderWindow* QOEngine::getRenderWindow()
{
    return mOgreWindow;
}

bool QOEngine::isStarted() const
{
    return mRoot;
}

void QOEngine::startEngine()
{
    activateOgreContext();

    QString basePath = QCoreApplication::applicationDirPath();

    mResourceConfig = (basePath + "/resources.cfg").toStdString();

    Ogre::Root *ogreRoot = new Ogre::Root("", mResourceConfig);

#if defined(Q_OS_MAC)
    basePath += "/../Plugins";
#endif

#if (defined(DEBUG) && defined(Q_OS_WIN)) // We dont use debug dynamic libraries on mac.
    const std::string renderSystemPath = (basePath + "/RenderSystem_GL_d").toStdString();
    const std::string octtreePath = (basePath + "/Plugin_OctreeSceneManager_d").toStdString();
#else
    const std::string renderSystemPath = (basePath + "/RenderSystem_GL").toStdString();
    const std::string octtreePath = (basePath + "/Plugin_OctreeSceneManager").toStdString();
#endif

    ogreRoot->loadPlugin(renderSystemPath);
    ogreRoot->loadPlugin(octtreePath);

    const Ogre::String renderSysName = "OpenGL Rendering Subsystem";
    Ogre::RenderSystem *renderSystem = ogreRoot->getRenderSystemByName(renderSysName);

#if defined(Q_OS_WIN)
    renderSystem->setConfigOption("FSAA","0");
#else
    renderSystem->setConfigOption("FSAA", "4");
#endif

    if(!renderSystem)
    {
        throw std::runtime_error("Render system not found. \
                                  The RenderSystemGL shared library \
                                  is most likely not being found.");
    }

    ogreRoot->setRenderSystem(renderSystem);
    ogreRoot->initialise(false);

    Ogre::NameValuePairList params;

    params["externalGLControl"] = "true";
    params["currentGLContext"] = "true";
    params["hidden"] = "true";

#if defined(Q_OS_MAC)
    params["macAPI"] = "cocoa";
    params["macAPICocoaUseNSView"] = "true";
#endif

    //Finally create our window.
    mOgreWindow = ogreRoot->createRenderWindow("OgreWindow", 1, 1, false, &params);
    mOgreWindow->setActive(true);
    mOgreWindow->setVisible(false);
    mOgreWindow->update(false);

    Ogre::Animation::setDefaultInterpolationMode(Ogre::Animation::IM_LINEAR);
    Ogre::Animation::setDefaultRotationInterpolationMode(Ogre::Animation::RIM_LINEAR);

    doneOgreContext();

    mRoot = ogreRoot;
}

void QOEngine::stopEngine()
{
    if (mRoot)
    {
//        m_root->detachRenderTarget(m_renderTexture);
        // TODO tell node(s) to detach
        delete mRoot;
        mRoot = NULL;
    }
}

QQuickWindow* QOEngine::getQQuickWindow()
{
    return mQuickWindow;
}

void QOEngine::setQuickWindow(QQuickWindow *window)
{
    Q_ASSERT(window);

    mQuickWindow = window;

    mOgreContext = QOpenGLContext::currentContext();
    mOgreContext->doneCurrent();
    mOgreContext->makeCurrent(mQuickWindow);
}

void QOEngine::activateOgreContext()
{
    mQuickWindow->resetOpenGLState();
}

void QOEngine::doneOgreContext()
{
    mQuickWindow->resetOpenGLState();
}

QOpenGLContext* QOEngine::ogreContext() const
{
    return mOgreContext;
}

QSGTexture* QOEngine::createTextureFromId(uint id,
                                            const QSize &size,
                                            QQuickWindow::CreateTextureOptions options) const
{
    return mQuickWindow->createTextureFromId(id, size, options);
}

void QOEngine::setupResources(void)
{
    QString basePath = QCoreApplication::applicationDirPath();

    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourceConfig);

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

            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
