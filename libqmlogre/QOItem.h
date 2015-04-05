/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#ifndef QOItem_H
#define QOItem_H

#include "qmlogre_global.h"
#include <QtQuick/QQuickItem>

class QOEngine;
class QONode;
class QOCamera;

class DLL_EXPORT QOItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QObject* camera READ camera WRITE setCamera)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)
    Q_PROPERTY(QOEngine* engine READ engine WRITE setEngine)

public:
    QOItem(QQuickItem* parent = 0);

    QObject* camera() const;
    void setCamera(QObject* camera);

    // QImage's "isNull" returns true if the allocation failed.
    QImage saveCurrentImage();

    QColor backgroundColor() const;
    void setBackgroundColor(QColor color);

    QOEngine* engine() const;
    void setEngine(QOEngine* QOEngine);

protected:
    virtual QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

private slots:
    void windowChanged(QQuickWindow* window);

private:
    QColor mBackgroundColor;
    QOCamera* mCamera;
    QONode* mLastNode;
    QOEngine* mEngineItem;
};

#endif // QOItem_H
