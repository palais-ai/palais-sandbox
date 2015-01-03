/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#ifndef OGREITEM_H
#define OGREITEM_H

#include <QtQuick/QQuickItem>

class OgreEngine;
class OgreNode;
class CameraNodeObject;

class OgreItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QObject* camera READ camera WRITE setCamera)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(OgreEngine* ogreEngine READ ogreEngine WRITE setOgreEngine)

public:
    OgreItem(QQuickItem* parent = 0);

    QObject* camera() const;
    void setCamera(QObject* camera);

    QColor backgroundColor() const;
    void setBackgroundColor(QColor color);

    OgreEngine* ogreEngine() const;
    void setOgreEngine(OgreEngine* ogreEngine);

protected:
    virtual QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

private slots:
    void windowChanged(QQuickWindow* window);

private:
    QColor m_backgroundColor;
    CameraNodeObject* m_camera;
    OgreNode* mLastNode;
    OgreEngine* m_ogreEngineItem;
};

#endif // OGREITEM_H
