/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#ifndef OGREENGINEITEM_H
#define OGREENGINEITEM_H

#include <OgreString.h>
#include <QObject>
#include <QQuickWindow>
#include <QOpenGLContext>
#include <OgreResourceGroupManager.h>

namespace Ogre {
class Root;
class RenderTexture;
class Viewport;
class RenderTarget;
class RenderWindow;
}

/**
 * @brief The OgreEngineItem class
 * Must only be constructed from within Qt QML rendering thread.
 */
class OgreEngine : public QObject, public Ogre::ResourceGroupListener
{
    Q_OBJECT
    Q_PROPERTY(qreal loadingProgress READ loadingProgress NOTIFY onLoadingProgressChanged)

public:
    OgreEngine(QQuickWindow *window = 0);
    ~OgreEngine();
    Ogre::Root *startEngine();
    void stopEngine(Ogre::Root *ogreRoot);

    void activateOgreContext();
    void doneOgreContext();

    void lockEngine();
    void unlockEngine();

    QOpenGLContext* ogreContext() const;

    QSGTexture* createTextureFromId(uint id,
                                    const QSize &size,
                                    QQuickWindow::CreateTextureOptions options = QQuickWindow::CreateTextureOption(0)) const;

    void setupResources(void);

    qreal loadingProgress() const
    {
        return mLoadingProgress;
    }

    // ResourceGroupListener callbacks
    void resourceGroupScriptingStarted(const Ogre::String& groupName, size_t scriptCount);
    void scriptParseStarted(const Ogre::String& scriptName, bool &skipThisScript);
    void scriptParseEnded(const Ogre::String& scriptName, bool skipped);
    void resourceGroupScriptingEnded(const Ogre::String& groupName);
    void resourceGroupLoadStarted(const Ogre::String& groupName, size_t resourceCount);
    void resourceLoadStarted(const Ogre::ResourcePtr& resource);
    void resourceLoadEnded(void);
    void worldGeometryStageStarted(const Ogre::String& description);
    void worldGeometryStageEnded(void);
    void resourceGroupLoadEnded(const Ogre::String& groupName);

signals:
    void onLoadingProgressChanged(qreal progress);
private:
    void addLoadingProgress(qreal progress)
    {
        mLoadingProgress += progress;
        emit onLoadingProgressChanged(mLoadingProgress);
    }

    Ogre::String m_resources_cfg;
    Ogre::RenderWindow *m_ogreWindow;

    QQuickWindow *m_quickWindow;

    /** Pointer to QOpenGLContext to be used by Ogre. */
    QOpenGLContext* m_ogreContext;
    /** Pointer to QOpenGLContext to be restored after Ogre context. */
    QOpenGLContext* m_qtContext;

    // Loading progress variables
    qreal mLoadingProgress;
    qreal mCurrentProgress;
    qreal mInitProportion;
    int mNumGroupsInit, mNumGroupsLoad;
protected:
    void setQuickWindow(QQuickWindow *window);
};

#endif // OGREENGINEITEM_H
