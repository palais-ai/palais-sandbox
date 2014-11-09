/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "exampleapp.h"

#include "cameranodeobject.h"

#include "../libqmlogre/ogreitem.h"
#include "../libqmlogre/ogreengine.h"

#include <QCoreApplication>
#include <QtQml/QQmlContext>

ExampleApp::ExampleApp(QWindow *parent) :
    QQuickView(parent)
  , m_ogreEngine(0)
  , m_sceneManager(0)
  , m_root(0)
{
    qmlRegisterType<CameraNodeObject>("Example", 1, 0, "Camera");

    // start Ogre once we are in the rendering thread (Ogre must live in the rendering thread)
    connect(this, &ExampleApp::beforeRendering, this, &ExampleApp::initializeOgre, Qt::DirectConnection);
    connect(this, &ExampleApp::ogreInitialized, this, &ExampleApp::addContent);
}

ExampleApp::~ExampleApp()
{
    if (m_sceneManager) {
        m_root->destroySceneManager(m_sceneManager);
    }
}

void ExampleApp::initializeOgre()
{
    // we only want to initialize once
    disconnect(this, &ExampleApp::beforeRendering, this, &ExampleApp::initializeOgre);

    // start up Ogre
    m_ogreEngine = new OgreEngine(this);
    m_root = m_ogreEngine->startEngine();
    m_ogreEngine->setupResources();

    // set up Ogre scene
    m_sceneManager = m_root->createSceneManager(Ogre::ST_GENERIC, "mySceneManager");

    m_sceneManager->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.3));
    m_sceneManager->createLight("myLight")->setPosition(20, 80, 50);

    // Resources with textures must be loaded within Ogre's GL context
    m_ogreEngine->activateOgreContext();

    m_sceneManager->setSkyBox(true, "SpaceSkyBox", 10000);
    m_sceneManager->getRootSceneNode()->attachObject(m_sceneManager->createEntity("Head", "ogrehead.mesh"));

    m_ogreEngine->doneOgreContext();

    emit(ogreInitialized());
}

void ExampleApp::addContent()
{
    // expose objects as QML globals
    rootContext()->setContextProperty("Window", this);
    rootContext()->setContextProperty("OgreEngine", m_ogreEngine);

    // load the QML scene
    setResizeMode(QQuickView::SizeRootObjectToView);
    setSource(QUrl("qrc:/qml/example.qml"));
}
